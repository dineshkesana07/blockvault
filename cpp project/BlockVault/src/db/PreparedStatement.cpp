#include "db/PreparedStatement.hpp"
#include "common/Exceptions.hpp"
#include <sqlite3.h>

namespace blockvault {

void PreparedStatement::check_db_open() const {
    if (!m_db) {
        throw DatabaseException("Cannot prepare statement: database connection handle is null");
    }
}

void PreparedStatement::check_statement_valid() const {
    if (!m_stmt) {
        throw DatabaseException("Invalid statement operation: sqlite3_stmt handle is null for SQL: " + m_sql);
    }
}

PreparedStatement::PreparedStatement(sqlite3* db, const std::string& sql)
    : m_db(db), m_stmt(nullptr), m_sql(sql) 
{
    check_db_open();
    int rc = sqlite3_prepare_v2(m_db, m_sql.c_str(), -1, &m_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(m_db);
        throw DatabaseException("Failed to prepare SQL statement: " + err + " [SQL: " + m_sql + "]");
    }
}

PreparedStatement::~PreparedStatement() noexcept {
    if (m_stmt) {
        sqlite3_finalize(m_stmt);
        m_stmt = nullptr;
    }
}

PreparedStatement::PreparedStatement(PreparedStatement&& other) noexcept
    : m_db(other.m_db),
      m_stmt(other.m_stmt),
      m_sql(std::move(other.m_sql)) 
{
    other.m_db = nullptr;
    other.m_stmt = nullptr;
}

PreparedStatement& PreparedStatement::operator=(PreparedStatement&& other) noexcept {
    if (this != &other) {
        if (m_stmt) {
            sqlite3_finalize(m_stmt);
        }
        m_db = other.m_db;
        m_stmt = other.m_stmt;
        m_sql = std::move(other.m_sql);
        other.m_db = nullptr;
        other.m_stmt = nullptr;
    }
    return *this;
}

void PreparedStatement::bind_int(int index, int value) {
    check_statement_valid();
    int rc = sqlite3_bind_int(m_stmt, index, value);
    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to bind integer at parameter " + std::to_string(index) + ": " + sqlite3_errmsg(m_db));
    }
}

void PreparedStatement::bind_int64(int index, int64_t value) {
    check_statement_valid();
    int rc = sqlite3_bind_int64(m_stmt, index, value);
    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to bind int64 at parameter " + std::to_string(index) + ": " + sqlite3_errmsg(m_db));
    }
}

void PreparedStatement::bind_text(int index, const std::string& text) {
    check_statement_valid();
    int rc = sqlite3_bind_text(m_stmt, index, text.c_str(), static_cast<int>(text.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to bind text at parameter " + std::to_string(index) + ": " + sqlite3_errmsg(m_db));
    }
}

void PreparedStatement::bind_blob(int index, const ByteVector& blob) {
    bind_blob(index, blob.data(), blob.size());
}

void PreparedStatement::bind_blob(int index, const uint8_t* data, size_t size) {
    check_statement_valid();
    int rc = sqlite3_bind_blob(m_stmt, index, data, static_cast<int>(size), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to bind blob at parameter " + std::to_string(index) + ": " + sqlite3_errmsg(m_db));
    }
}

void PreparedStatement::bind_null(int index) {
    check_statement_valid();
    int rc = sqlite3_bind_null(m_stmt, index);
    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to bind null at parameter " + std::to_string(index) + ": " + sqlite3_errmsg(m_db));
    }
}

bool PreparedStatement::step() {
    check_statement_valid();
    int rc = sqlite3_step(m_stmt);
    if (rc == SQLITE_ROW) {
        return true;
    }
    if (rc == SQLITE_DONE) {
        return false;
    }
    std::string err = sqlite3_errmsg(m_db);
    throw DatabaseException("SQLite step failed (code " + std::to_string(rc) + "): " + err + " [SQL: " + m_sql + "]");
}

void PreparedStatement::execute() {
    check_statement_valid();
    int rc = sqlite3_step(m_stmt);
    if (rc != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(m_db);
        throw DatabaseException("SQLite execute failed (code " + std::to_string(rc) + "): " + err + " [SQL: " + m_sql + "]");
    }
}

void PreparedStatement::reset() {
    if (m_stmt) {
        sqlite3_reset(m_stmt);
    }
}

void PreparedStatement::clear_bindings() {
    if (m_stmt) {
        sqlite3_clear_bindings(m_stmt);
    }
}

int PreparedStatement::get_int(int column) const {
    check_statement_valid();
    return sqlite3_column_int(m_stmt, column);
}

int64_t PreparedStatement::get_int64(int column) const {
    check_statement_valid();
    return sqlite3_column_int64(m_stmt, column);
}

std::string PreparedStatement::get_text(int column) const {
    check_statement_valid();
    const unsigned char* text = sqlite3_column_text(m_stmt, column);
    if (!text) return "";
    int bytes = sqlite3_column_bytes(m_stmt, column);
    return std::string(reinterpret_cast<const char*>(text), static_cast<size_t>(bytes));
}

ByteVector PreparedStatement::get_blob(int column) const {
    check_statement_valid();
    const void* blob = sqlite3_column_blob(m_stmt, column);
    int bytes = sqlite3_column_bytes(m_stmt, column);
    if (!blob || bytes <= 0) return {};
    const uint8_t* p = static_cast<const uint8_t*>(blob);
    return ByteVector(p, p + bytes);
}

bool PreparedStatement::is_null(int column) const {
    check_statement_valid();
    return sqlite3_column_type(m_stmt, column) == SQLITE_NULL;
}

int PreparedStatement::column_count() const {
    check_statement_valid();
    return sqlite3_column_count(m_stmt);
}

} // namespace blockvault

