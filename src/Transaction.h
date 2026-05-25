#pragma once

#include <memory>
#include <mutex>

class Database;

// RAII: BEGIN al crear, COMMIT al destruir si no hubo rollback.
class Transaction {
public:
    explicit Transaction(Database& db);
    ~Transaction();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    void commit();
    void rollback() noexcept;

private:
    Database* db_{ nullptr };
    std::unique_lock<std::recursive_mutex> lock_;
    bool active_{ false };
    bool committed_{ false };
};
