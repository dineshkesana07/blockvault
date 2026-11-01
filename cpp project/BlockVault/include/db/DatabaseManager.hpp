#pragma once

#include "models/User.hpp"
#include "models/Credential.hpp"
#include "models/Block.hpp"
#include "db/DatabaseEntities.hpp"
#include "db/PreparedStatement.hpp"
#include "db/Transaction.hpp"
#include <string>
#include <vector>
#include <optional>
#include <memory>

struct sqlite3;

namespace blockvault {

/**
 * @class DatabaseManager
 * @brief Thread-safe SQLite persistence engine engineered with modern C++20 and RAII.
 * 
 * Manages database connections, Write-Ahead Logging (WAL) pragmas, foreign keys,
 * schema migrations, prepared SQL statements, atomic transactions, and CRUD operations.
 */
class DatabaseManager {
private:
    sqlite3*    m_db{nullptr};
    std::string m_db_path;
    bool        m_is_open{false};

    void configure_pragmas();
    void apply_migrations();

public:
    DatabaseManager();
    explicit DatabaseManager(const std::string& db_path);
    ~DatabaseManager() noexcept;

    // Non-copyable (Unique database handle ownership)
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Moveable
    DatabaseManager(DatabaseManager&& other) noexcept;
    DatabaseManager& operator=(DatabaseManager&& other) noexcept;

    // Connection lifecycle
    void open(const std::string& db_path);
    void close() noexcept;
    [[nodiscard]] bool is_open() const noexcept { return m_is_open && m_db != nullptr; }
    [[nodiscard]] const std::string& path() const noexcept { return m_db_path; }
    [[nodiscard]] sqlite3* handle() const noexcept { return m_db; }

    // Direct SQL execution helper
    void execute_raw(const std::string& sql);

    // --- Schema Migration Helpers ---
    [[nodiscard]] int get_schema_version();

    // --- RAII Transaction Helper ---
    [[nodiscard]] Transaction create_transaction();

    // --- Core Entity CRUD Operations (users, credentials, audit_records) ---

    // 1. Users Table Operations
    void insert_user_entity(const UserEntity& user);
    [[nodiscard]] std::optional<UserEntity> get_user_entity_by_id(const std::string& user_id);
    [[nodiscard]] std::optional<UserEntity> get_user_entity_by_username(const std::string& username);
    void update_user_entity_verifier(const std::string& user_id, const ByteVector& salt, const ByteVector& verifier);
    bool delete_user_entity(const std::string& user_id);

    // 2. Credentials Table Operations
    void insert_credential_entity(const CredentialEntity& credential);
    [[nodiscard]] std::optional<CredentialEntity> get_credential_entity_by_id(
        const std::string& credential_id,
        const std::string& user_id
    );
    [[nodiscard]] std::vector<CredentialEntity> get_credential_entities_for_user(const std::string& user_id);
    void update_credential_entity(const CredentialEntity& credential);
    bool delete_credential_entity(const std::string& credential_id, const std::string& user_id);
    [[nodiscard]] size_t get_credential_entity_count(const std::string& user_id);

    // 3. Audit Records Table Operations
    int64_t insert_audit_record(const AuditRecordEntity& record);
    [[nodiscard]] std::optional<AuditRecordEntity> get_last_audit_record();
    [[nodiscard]] std::vector<AuditRecordEntity> get_all_audit_records();
    [[nodiscard]] std::vector<AuditRecordEntity> get_audit_records_by_event(const std::string& event_type);
    [[nodiscard]] uint64_t get_audit_record_count();

    // --- High-Level Domain Model Compatibility Adapters ---
    void insert_user(const User& user);
    [[nodiscard]] std::optional<User> get_user_by_id(const std::string& user_id);
    [[nodiscard]] std::optional<User> get_user_by_email(const std::string& email);
    void update_user(const User& user);
    bool delete_user(const std::string& user_id);

    void insert_credential(const Credential& credential);
    [[nodiscard]] std::optional<Credential> get_credential_by_id(
        const std::string& credential_id,
        const std::string& user_id
    );
    [[nodiscard]] std::vector<Credential> get_credentials_for_user(const std::string& user_id);
    void update_credential(const Credential& credential);
    bool delete_credential(const std::string& credential_id, const std::string& user_id);
    [[nodiscard]] size_t get_credential_count(const std::string& user_id);

    void insert_block(const Block& block);
    [[nodiscard]] std::optional<Block> get_last_block();
    [[nodiscard]] std::vector<Block> get_all_blocks();
    [[nodiscard]] std::vector<Block> get_blocks_for_user(const std::string& user_id);
    [[nodiscard]] uint64_t get_block_count();

    // --- Session Persistence ---
    void insert_session(
        const std::string& token,
        const std::string& user_id,
        const std::string& client_ip,
        int64_t expires_at
    );
    [[nodiscard]] std::optional<std::string> validate_session(const std::string& token);
    void delete_session(const std::string& token);
    void delete_user_sessions(const std::string& user_id);
    void purge_expired_sessions();
};

} // namespace blockvault
