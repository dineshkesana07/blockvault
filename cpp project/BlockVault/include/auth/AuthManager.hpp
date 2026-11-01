#pragma once

#include "models/User.hpp"
#include "db/DatabaseManager.hpp"
#include "auth/SessionManager.hpp"
#include "ledger/AuditLedger.hpp"
#include <memory>
#include <string>

namespace blockvault {

/**
 * @class AuthManager
 * @brief High-level domain service orchestrating zero-knowledge authentication.
 * 
 * Coordinates user registration, master password verification, envelope key
 * derivation (KEK unwraps DEK), session lifecycle, and cryptographic audit logging.
 */
class AuthManager {
private:
    std::shared_ptr<DatabaseManager> m_db;
    std::shared_ptr<SessionManager>  m_session_mgr;
    std::shared_ptr<AuditLedger>     m_ledger;

public:
    AuthManager(
        std::shared_ptr<DatabaseManager> db,
        std::shared_ptr<SessionManager>  session_mgr,
        std::shared_ptr<AuditLedger>     ledger
    );
    ~AuthManager() = default;

    // Non-copyable
    AuthManager(const AuthManager&) = delete;
    AuthManager& operator=(const AuthManager&) = delete;

    // Moveable
    AuthManager(AuthManager&&) noexcept = default;
    AuthManager& operator=(AuthManager&&) noexcept = default;

    // Register a new user account with zero-knowledge envelope encryption
    User register_user(const std::string& email, const std::string& master_password);

    // Authenticate user with master password and create unlocked session
    std::string login(
        const std::string& email,
        const std::string& master_password,
        const std::string& client_ip = "127.0.0.1"
    );

    // Explicitly unlock an existing session's vault using master password
    void unlock_vault(const std::string& token, const std::string& master_password);

    // Lock active session vault (wiping DEK from memory)
    void lock_vault(const std::string& token);

    // Terminate session on user logout
    void logout(const std::string& token);

    // Retrieve the authenticated user associated with an active token
    [[nodiscard]] User get_authenticated_user(const std::string& token);
};

} // namespace blockvault

