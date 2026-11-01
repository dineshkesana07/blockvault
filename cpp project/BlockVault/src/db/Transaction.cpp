#include "db/Transaction.hpp"
#include "common/Exceptions.hpp"
#include <sqlite3.h>
#include <string>

namespace blockvault {

void Transaction::execute_sql(const char* sql) {
    if (!m_db) {
        throw DatabaseException("Cannot execute transaction SQL: database handle is null");
    }
    char* err_msg = nullptr;
    int rc = sqlite3_exec(m_db, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "Unknown error";
        sqlite3_free(err_msg);
        throw DatabaseException("Transaction SQL failed (" + std::string(sql) + "): " + err);
    }
}

Transaction::Transaction(sqlite3* db) : m_db(db), m_committed(false), m_active(false) {
    if (!m_db) {
        throw DatabaseException("Cannot begin transaction: null database handle");
    }
    execute_sql("BEGIN IMMEDIATE TRANSACTION;");
    m_active = true;
}

Transaction::~Transaction() noexcept {
    if (m_active && !m_committed && m_db) {
        try {
            execute_sql("ROLLBACK;");
        } catch (...) {
            // Destructors must not throw exceptions
        }
    }
}

Transaction::Transaction(Transaction&& other) noexcept
    : m_db(other.m_db),
      m_committed(other.m_committed),
      m_active(other.m_active) 
{
    other.m_db = nullptr;
    other.m_committed = false;
    other.m_active = false;
}

Transaction& Transaction::operator=(Transaction&& other) noexcept {
    if (this != &other) {
        if (m_active && !m_committed && m_db) {
            try {
                execute_sql("ROLLBACK;");
            } catch (...) {}
        }
        m_db = other.m_db;
        m_committed = other.m_committed;
        m_active = other.m_active;
        other.m_db = nullptr;
        other.m_committed = false;
        other.m_active = false;
    }
    return *this;
}

void Transaction::commit() {
    if (!m_active) {
        throw DatabaseException("Cannot commit: transaction is not active");
    }
    if (m_committed) {
        return;
    }
    execute_sql("COMMIT;");
    m_committed = true;
    m_active = false;
}

void Transaction::rollback() {
    if (!m_active) {
        throw DatabaseException("Cannot rollback: transaction is not active");
    }
    if (m_committed) {
        throw DatabaseException("Cannot rollback: transaction was already committed");
    }
    execute_sql("ROLLBACK;");
    m_active = false;
}

} // namespace blockvault

