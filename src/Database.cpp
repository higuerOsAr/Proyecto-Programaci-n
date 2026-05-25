#include "Database.h"

#include "SqliteException.h"
#include "Statement.h"
#include "Transaction.h"

#include <sqlite3.h>

#include <utility>

namespace {

constexpr int kBusyTimeoutMs = 5000;
constexpr int kCachePages = -64000; // ~64 MB (paginas de 1 KB negativas = KB)

} // namespace

Database::Database(std::string path) : path_(std::move(path)) {
    open(path_);
    applyPerformancePragmas();
}

Database::~Database() {
    std::lock_guard lock(cacheMutex_);
    for (auto& [key, stmt] : stmtCache_) {
        (void)key;
        sqlite3_finalize(stmt);
    }
    stmtCache_.clear();
    close();
}

Database::Database(Database&& other) noexcept
    : db_(other.db_)
    , path_(std::move(other.path_))
    , stmtCache_(std::move(other.stmtCache_)) {
    other.db_ = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        std::lock_guard lock(cacheMutex_);
        for (auto& [key, stmt] : stmtCache_) {
            (void)key;
            sqlite3_finalize(stmt);
        }
        stmtCache_.clear();
        close();

        db_ = other.db_;
        path_ = std::move(other.path_);
        stmtCache_ = std::move(other.stmtCache_);
        other.db_ = nullptr;
    }
    return *this;
}

void Database::open(const std::string& path) {
    const int rc = sqlite3_open_v2(
        path.c_str(),
        &db_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
        nullptr);

    if (rc != SQLITE_OK) {
        const std::string msg = db_
            ? sqlite3_errmsg(db_)
            : "No se pudo abrir la base de datos.";
        close();
        throw SqliteException(msg, rc);
    }

    sqlite3_busy_timeout(db_, kBusyTimeoutMs);
}

void Database::close() noexcept {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void Database::throwOnError(int rc, const char* context) const {
    if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW) {
        return;
    }
    throw SqliteException(
        std::string(context) + ": " + sqlite3_errmsg(db_),
        rc);
}

void Database::applyPerformancePragmas() {
    // WAL: lecturas concurrentes sin bloquear escrituras.
    // synchronous=NORMAL: buen equilibrio durabilidad/velocidad con WAL.
    // cache_size + temp_store: menos I/O en disco.
    // mmap_size: lecturas grandes mas rapidas cuando el SO lo permite.
    exec("PRAGMA journal_mode = WAL;");
    exec("PRAGMA synchronous = NORMAL;");
    exec("PRAGMA foreign_keys = ON;");
    exec("PRAGMA temp_store = MEMORY;");
    exec("PRAGMA cache_size = " + std::to_string(kCachePages) + ";");
    exec("PRAGMA mmap_size = 268435456;"); // 256 MB
    exec("ANALYZE;");
}

void Database::exec(const std::string& sql) {
    std::lock_guard lock(*dbMutex_);
    execUnlocked(sql);
}

void Database::execUnlocked(const std::string& sql) {
    char* errMsg = nullptr;
    const int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string msg = errMsg ? errMsg : "Error desconocido en exec.";
        sqlite3_free(errMsg);
        throw SqliteException(msg, rc);
    }
}

Statement Database::prepare(const std::string& sql) {
    std::lock_guard lock(*dbMutex_);
    sqlite3_stmt* raw = nullptr;
    const int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &raw, nullptr);
    if (rc != SQLITE_OK) {
        throwOnError(rc, "prepare");
    }
    return Statement(raw, false, dbMutex_);
}

Statement Database::prepareCached(const std::string& key, const std::string& sql) {
    std::lock_guard cacheLock(cacheMutex_);
    auto it = stmtCache_.find(key);
    if (it == stmtCache_.end()) {
        sqlite3_stmt* raw = nullptr;
        const int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &raw, nullptr);
        if (rc != SQLITE_OK) {
            throwOnError(rc, "prepareCached");
        }
        it = stmtCache_.emplace(key, raw).first;
    } else {
        sqlite3_reset(it->second);
        sqlite3_clear_bindings(it->second);
    }
    return Statement(it->second, true, dbMutex_);
}

Transaction Database::beginTransaction() {
    return Transaction(*this);
}

std::uint64_t Database::lastInsertRowId() const {
    std::lock_guard lock(*dbMutex_);
    return static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db_));
}

std::uint64_t Database::changes() const {
    std::lock_guard lock(*dbMutex_);
    return static_cast<std::uint64_t>(sqlite3_changes(db_));
}
