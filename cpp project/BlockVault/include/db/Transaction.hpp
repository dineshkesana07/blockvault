#pragma once

struct sqlite3;

namespace blockvault {

/**
 * @class Transaction
 * @brief RAII transaction guard for SQLite operations.
 * 
 * Ensures atomic multi-step operations. Automatically executes a ROLLBACK upon
 * destruction unless commit() has been explicitly called, guaranteeing that
 * unexpected exceptions or errors leave the database in a consistent state.
 */
class Transaction {
private:
    sqlite3* m_db{nullptr};
    bool     m_committed{false};
    bool     m_active{false};

    void execute_sql(const char* sql);

public:
    explicit Transaction(sqlite3* db);
    ~Transaction() noexcept;

    // Non-copyable (Unique transaction scope)
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    // Moveable
    Transaction(Transaction&& other) noexcept;
    Transaction& operator=(Transaction&& other) noexcept;

    // Explicitly commit all changes in this transaction
    void commit();

    // Explicitly abort and revert all changes in this transaction
    void rollback();

    [[nodiscard]] bool is_active() const noexcept { return m_active; }
    [[nodiscard]] bool is_committed() const noexcept { return m_committed; }
};

} // namespace blockvault

