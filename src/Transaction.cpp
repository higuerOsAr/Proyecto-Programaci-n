#include "Transaction.h"

#include "Database.h"

Transaction::Transaction(Database& db)
    : db_(&db)
    , lock_(db.connectionMutex()) {
    db_->execUnlocked("BEGIN IMMEDIATE;");
    active_ = true;
}

Transaction::~Transaction() {
    if (active_ && !committed_) {
        rollback();
    }
}

void Transaction::commit() {
    if (active_ && !committed_) {
        db_->execUnlocked("COMMIT;");
        committed_ = true;
        active_ = false;
        lock_.unlock();
    }
}

void Transaction::rollback() noexcept {
    if (active_ && !committed_) {
        try {
            db_->execUnlocked("ROLLBACK;");
        } catch (...) {
        }
        active_ = false;
        if (lock_.owns_lock()) {
            lock_.unlock();
        }
    }
}
