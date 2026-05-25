#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

struct sqlite3_stmt;

// RAII sobre sqlite3_stmt. Si cached=true, no hace finalize (lo gestiona Database).
class Statement {
public:
    Statement(sqlite3_stmt* stmt, bool cached,
        std::shared_ptr<std::recursive_mutex> dbMutex);
    ~Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

    void bind(int index, int value);
    void bind(int index, std::int64_t value);
    void bind(int index, double value);
    void bind(int index, const std::string& value);
    void bindNull(int index);

    bool step();
    void reset();

    int columnInt(int index) const;
    std::int64_t columnInt64(int index) const;
    double columnDouble(int index) const;
    std::string columnText(int index) const;

    void run(); // step una vez y espera DONE (INSERT/UPDATE/DELETE)

private:
    sqlite3_stmt* stmt_{ nullptr };
    bool cached_{ false };
    std::shared_ptr<std::recursive_mutex> dbMutex_;
};
