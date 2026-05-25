#include "Statement.h"

#include "SqliteException.h"

#include <sqlite3.h>

Statement::Statement(sqlite3_stmt* stmt, bool cached,
    std::shared_ptr<std::recursive_mutex> dbMutex)
    : stmt_(stmt)
    , cached_(cached)
    , dbMutex_(std::move(dbMutex)) {}

Statement::~Statement() {
    if (!stmt_) {
        return;
    }
    std::lock_guard lock(*dbMutex_);
    if (cached_) {
        sqlite3_reset(stmt_);
        sqlite3_clear_bindings(stmt_);
    } else {
        sqlite3_finalize(stmt_);
    }
}

Statement::Statement(Statement&& other) noexcept
    : stmt_(other.stmt_)
    , cached_(other.cached_)
    , dbMutex_(std::move(other.dbMutex_)) {
    other.stmt_ = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (stmt_ && !cached_) {
            std::lock_guard lock(*dbMutex_);
            sqlite3_finalize(stmt_);
        }
        stmt_ = other.stmt_;
        cached_ = other.cached_;
        dbMutex_ = std::move(other.dbMutex_);
        other.stmt_ = nullptr;
    }
    return *this;
}

void Statement::bind(int index, int value) {
    std::lock_guard lock(*dbMutex_);
    const int rc = sqlite3_bind_int(stmt_, index, value);
    if (rc != SQLITE_OK) {
        throw SqliteException(sqlite3_errmsg(sqlite3_db_handle(stmt_)), rc);
    }
}

void Statement::bind(int index, std::int64_t value) {
    std::lock_guard lock(*dbMutex_);
    const int rc = sqlite3_bind_int64(stmt_, index, value);
    if (rc != SQLITE_OK) {
        throw SqliteException(sqlite3_errmsg(sqlite3_db_handle(stmt_)), rc);
    }
}

void Statement::bind(int index, double value) {
    std::lock_guard lock(*dbMutex_);
    const int rc = sqlite3_bind_double(stmt_, index, value);
    if (rc != SQLITE_OK) {
        throw SqliteException(sqlite3_errmsg(sqlite3_db_handle(stmt_)), rc);
    }
}

void Statement::bind(int index, const std::string& value) {
    std::lock_guard lock(*dbMutex_);
    const int rc = sqlite3_bind_text(
        stmt_, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        throw SqliteException(sqlite3_errmsg(sqlite3_db_handle(stmt_)), rc);
    }
}

void Statement::bindNull(int index) {
    std::lock_guard lock(*dbMutex_);
    const int rc = sqlite3_bind_null(stmt_, index);
    if (rc != SQLITE_OK) {
        throw SqliteException(sqlite3_errmsg(sqlite3_db_handle(stmt_)), rc);
    }
}

bool Statement::step() {
    std::lock_guard lock(*dbMutex_);
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) {
        return true;
    }
    if (rc == SQLITE_DONE) {
        return false;
    }
    throw SqliteException(sqlite3_errmsg(sqlite3_db_handle(stmt_)), rc);
}

void Statement::reset() {
    std::lock_guard lock(*dbMutex_);
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
}

void Statement::run() {
    step();
}

int Statement::columnInt(int index) const {
    std::lock_guard lock(*dbMutex_);
    return sqlite3_column_int(stmt_, index);
}

std::int64_t Statement::columnInt64(int index) const {
    std::lock_guard lock(*dbMutex_);
    return sqlite3_column_int64(stmt_, index);
}

double Statement::columnDouble(int index) const {
    std::lock_guard lock(*dbMutex_);
    return sqlite3_column_double(stmt_, index);
}

std::string Statement::columnText(int index) const {
    std::lock_guard lock(*dbMutex_);
    const unsigned char* text = sqlite3_column_text(stmt_, index);
    return text ? reinterpret_cast<const char*>(text) : "";
}
