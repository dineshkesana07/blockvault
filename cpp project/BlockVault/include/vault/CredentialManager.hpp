#pragma once

#include "models/Credential.hpp"
#include "models/Vault.hpp"
#include "db/DatabaseManager.hpp"
#include "auth/SessionManager.hpp"
#include "ledger/AuditLedger.hpp"
#include <memory>
#include <string>
#include <vector>

namespace blockvault {

struct DecryptedCredential {
    std::string        id;
    std::string        title;
    std::string        website_url;
    std::string        username;
    std::string        password;    // Plaintext password in ephemeral memory
    std::string        notes;       // Plaintext notes in ephemeral memory
    CredentialCategory category;
    bool               favorite{false};
    Timestamp          created_at;
    Timestamp          updated_at;
};

/**
 * @class CredentialManager
 * @brief High-level service managing encrypted credential lifecycle and tenant isolation.
 * 
 * Enforces authenticated session boundaries, performs cryptographic encryption/decryption
 * with in-memory DEK, persists encrypted records to SQLite, and maintains audit trails.
 */
class CredentialManager {
private:
    std::shared_ptr<DatabaseManager> m_db;
    std::shared_ptr<SessionManager>  m_session_mgr;
    std::shared_ptr<AuditLedger>     m_ledger;

public:
    CredentialManager(
        std::shared_ptr<DatabaseManager> db,
        std::shared_ptr<SessionManager>  session_mgr,
        std::shared_ptr<AuditLedger>     ledger
    );
    ~CredentialManager() = default;

    // Non-copyable
    CredentialManager(const CredentialManager&) = delete;
    CredentialManager& operator=(const CredentialManager&) = delete;

    // Moveable
    CredentialManager(CredentialManager&&) noexcept = default;
    CredentialManager& operator=(CredentialManager&&) noexcept = default;

    // Create and encrypt a new credential record
    Credential create_credential(
        const std::string& token,
        const std::string& title,
        const std::string& website_url,
        const std::string& username,
        const std::string& plaintext_password,
        const std::string& plaintext_notes = "",
        CredentialCategory category = CredentialCategory::Login,
        bool favorite = false
    );

    // Retrieve and decrypt a single credential
    [[nodiscard]] DecryptedCredential get_decrypted_credential(
        const std::string& token,
        const std::string& credential_id
    );

    // Retrieve the user's populated Vault container (encrypted payloads intact)
    [[nodiscard]] Vault get_vault(const std::string& token);

    // Update an existing credential record
    void update_credential(
        const std::string& token,
        const std::string& credential_id,
        const std::string& title,
        const std::string& website_url,
        const std::string& username,
        const std::string& plaintext_password,
        const std::string& plaintext_notes = "",
        CredentialCategory category = CredentialCategory::Login,
        bool favorite = false
    );

    // Delete a credential record
    bool delete_credential(const std::string& token, const std::string& credential_id);

    // Toggle favorite state
    void toggle_favorite(const std::string& token, const std::string& credential_id);
};

} // namespace blockvault

