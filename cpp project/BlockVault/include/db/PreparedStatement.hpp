#pragma once

#include "common/Types.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

struct sqlite3;
struct sqlite3_stmt;

namespace blockvault {

/**
 * @class PreparedStatement
 * @brief RAII wrapper for SQLite prepared statements (sqlite3_stmt*).
 * 
 * Guarantees statement finalization upon scope exit, eliminates SQL injection
 * vulnerabilities via strongly typed parameter bindings, and provides safe,
 * bounds-checked column retrieval.
 */
class PreparedStatement {
private:
    sqlite3*      m_db{nullptr};
    sqlite3_stmt* m_stmt{nullptr};
    std::string   m_sql;

    void check_db_open() const;
    void check_statement_valid() const;

public:
    PreparedStatement(sqlite3* db, const std::string& sql);
    ~PreparedStatement() noexcept;

    // Non-copyable (Unique ownership of sqlite3_stmt resource)
    PreparedStatement(const PreparedStatement&) = delete;
    PreparedStatement& operator=(const PreparedStatement&) = delete;

    // Move-constructible and move-assignable
    PreparedStatement(PreparedStatement&& other) noexcept;
    PreparedStatement& operator=(PreparedStatement&& other) noexcept;

    // --- Parameter Binding (1-based indices) ---
    void bind_int(int index, int value);
    void bind_int64(int index, int64_t value);
    void bind_text(int index, const std::string& text);
    void bind_blob(int index, const ByteVector& blob);
    void bind_blob(int index, const uint8_t* data, size_t size);
    void bind_null(int index);

    // --- Execution ---
    // Returns true if a row is available (SQLITE_ROW), false if finished (SQLITE_DONE)
    bool step();

    // Executes the statement expecting SQLITE_DONE (INSERT, UPDATE, DELETE)
    void execute();

    // Resets statement and clears bindings for reuse
    void reset();
    void clear_bindings();

    // --- Column Value Retrieval (0-based indices) ---
    [[nodiscard]] int get_int(int column) const;
    [[nodiscard]] int64_t get_int64(int column) const;
    [[nodiscard]] std::string get_text(int column) const;
    [[nodiscard]] ByteVector get_blob(int column) const;
    [[nodiscard]] bool is_null(int column) const;
    [[nodiscard]] int column_count() const;

    // Observers
    [[nodiscard]] sqlite3_stmt* get() const noexcept { return m_stmt; }
    [[nodiscard]] const std::string& sql() const noexcept { return m_sql; }
};

} // namespace blockvault

