#pragma once

#include "crypto/SecureBuffer.hpp"
#include "db/DatabaseManager.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <optional>

namespace blockvault {

struct ActiveSession {
    std::string  token;
    std::string  user_id;
    std::string  client_ip;
    Timestamp    created_at;
    Timestamp    last_active;
    Timestamp    expires_at;
    bool         is_unlocked{false};
    SecureBuffer active_dek; // Decrypted DEK kept strictly in volatile memory
};

/**
 * @class SessionManager
 * @brief Thread-safe session lifecycle and in-memory cryptographic enclave manager.
 * 
 * Manages active user sessions, handles vault lock/unlock states, and guards
 * the decrypted Data Encryption Key (DEK) in memory with automatic inactivity timeouts.
 */
class SessionManager {
private:
    std::shared_ptr<DatabaseManager> m_db;
    std::unordered_map<std::string, ActiveSession> m_active_sessions;
    int64_t m_inactivity_timeout_seconds{900}; // 15 minutes default

public:
    explicit SessionManager(
        std::shared_ptr<DatabaseManager> db,
        int64_t inactivity_timeout_seconds = 900
    );
    ~SessionManager() = default;

    // Non-copyable
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    // Moveable
    SessionManager(SessionManager&&) noexcept = default;
    SessionManager& operator=(SessionManager&&) noexcept = default;

    // Create a new authenticated session
    std::string create_session(
        const std::string& user_id,
        const std::string& client_ip,
        int64_t duration_seconds = 86400 // 24 hours total session duration
    );

    // Validate a session token; returns user ID if active and unexpired
    [[nodiscard]] std::optional<std::string> validate_session(const std::string& token);

    // Vault cryptographic locking and unlocking
    void unlock_vault(const std::string& token, SecureBuffer dek);
    void lock_vault(const std::string& token);
    [[nodiscard]] bool is_vault_unlocked(const std::string& token);

    // Access the in-memory decrypted DEK
    [[nodiscard]] const SecureBuffer& get_dek(const std::string& token);

    // Terminate and destroy session
    void terminate_session(const std::string& token);
    void terminate_all_user_sessions(const std::string& user_id);

    // Housekeeping
    void purge_inactive_sessions();
    [[nodiscard]] size_t active_session_count() const noexcept { return m_active_sessions.size(); }
};

} // namespace blockvault

