#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

struct sqlite3;
struct sqlite3_stmt;

class Statement;
class Transaction;

// Conexion SQLite con pragmas de rendimiento y cache de prepared statements.
class Database {
public:
    explicit Database(std::string path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    sqlite3* handle() const noexcept { return db_; }

    void exec(const std::string& sql);
    void applyPerformancePragmas();

    // Reutiliza statements preparados por clave (evita sqlite3_prepare_v2 repetido).
    Statement prepareCached(const std::string& key, const std::string& sql);
    Statement prepare(const std::string& sql);

    Transaction beginTransaction();

    std::uint64_t lastInsertRowId() const;
    std::uint64_t changes() const;

    // Un mutex por conexion: seguro compartir Database entre hilos.
    std::recursive_mutex& connectionMutex() const noexcept { return *dbMutex_; }

    void execUnlocked(const std::string& sql);

private:
    void open(const std::string& path);
    void close() noexcept;
    void throwOnError(int rc, const char* context) const;

    friend class Transaction;

    sqlite3* db_{ nullptr };
    std::string path_;
    std::unordered_map<std::string, sqlite3_stmt*> stmtCache_;
    mutable std::mutex cacheMutex_;
    std::shared_ptr<std::recursive_mutex> dbMutex_{ std::make_shared<std::recursive_mutex>() };
};
