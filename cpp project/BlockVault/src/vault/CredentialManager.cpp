#include "vault/CredentialManager.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"

namespace blockvault {

CredentialManager::CredentialManager(
    std::shared_ptr<DatabaseManager> db,
    std::shared_ptr<SessionManager>  session_mgr,
    std::shared_ptr<AuditLedger>     ledger)
    : m_db(std::move(db)),
      m_session_mgr(std::move(session_mgr)),
      m_ledger(std::move(ledger)) 
{
    if (!m_db || !m_session_mgr || !m_ledger) {
        throw VaultException("Dependencies cannot be null for CredentialManager");
    }
}

Credential CredentialManager::create_credential(
    const std::string& token,
    const std::string& title,
    const std::string& website_url,
    const std::string& username,
    const std::string& plaintext_password,
    const std::string& plaintext_notes,
    CredentialCategory category,
    bool favorite) 
{
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    if (!uid.has_value()) {
        throw AuthException("Unauthorized session for credential creation");
    }

    // Retrieve active Data Encryption Key from session enclave
    const SecureBuffer& dek = m_session_mgr->get_dek(token);

    std::string cred_id = CryptoManager::generate_uuid_v4();

    // Authenticated encryption of sensitive fields
    EncryptedPayload enc_password = CryptoManager::encrypt_string(
        plaintext_password, dek, "CRED_PASSWORD:" + cred_id
    );

    EncryptedPayload enc_notes;
    if (!plaintext_notes.empty()) {
        enc_notes = CryptoManager::encrypt_string(
            plaintext_notes, dek, "CRED_NOTES:" + cred_id
        );
    }

    Credential cred(
        cred_id,
        *uid,
        title,
        website_url,
        username,
        enc_password,
        enc_notes,
        category,
        favorite
    );

    m_db->insert_credential(cred);

    // Audit log
    m_ledger->record_event(
        *uid,
        AuditEventType::CredentialCreated,
        cred_id,
        "Created credential record for " + title
    );

    return cred;
}

DecryptedCredential CredentialManager::get_decrypted_credential(
    const std::string& token,
    const std::string& credential_id) 
{
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    if (!uid.has_value()) {
        throw AuthException("Unauthorized session for credential decryption");
    }

    const SecureBuffer& dek = m_session_mgr->get_dek(token);

    std::optional<Credential> cred = m_db->get_credential_by_id(credential_id, *uid);
    if (!cred.has_value()) {
        throw VaultException("Credential not found or access denied");
    }

    std::string decrypted_pass = CryptoManager::decrypt_string(
        cred->encrypted_password(), dek, "CRED_PASSWORD:" + credential_id
    );

    std::string decrypted_notes;
    if (!cred->encrypted_notes().ciphertext.empty()) {
        decrypted_notes = CryptoManager::decrypt_string(
            cred->encrypted_notes(), dek, "CRED_NOTES:" + credential_id
        );
    }

    DecryptedCredential result;
    result.id = cred->id();
    result.title = cred->title();
    result.website_url = cred->website_url();
    result.username = cred->username();
    result.password = std::move(decrypted_pass);
    result.notes = std::move(decrypted_notes);
    result.category = cred->category();
    result.favorite = cred->is_favorite();
    result.created_at = cred->created_at();
    result.updated_at = cred->updated_at();

    // Audit log read event
    m_ledger->record_event(
        *uid,
        AuditEventType::CredentialRead,
        credential_id,
        "Accessed sensitive credential details for " + cred->title()
    );

    return result;
}

Vault CredentialManager::get_vault(const std::string& token) {
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    if (!uid.has_value()) {
        throw AuthException("Unauthorized session for vault retrieval");
    }

    Vault vault(*uid);
    std::vector<Credential> list = m_db->get_credentials_for_user(*uid);
    for (const auto& cred : list) {
        vault.add_credential(cred);
    }

    return vault;
}

void CredentialManager::update_credential(
    const std::string& token,
    const std::string& credential_id,
    const std::string& title,
    const std::string& website_url,
    const std::string& username,
    const std::string& plaintext_password,
    const std::string& plaintext_notes,
    CredentialCategory category,
    bool favorite) 
{
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    if (!uid.has_value()) {
        throw AuthException("Unauthorized session for credential update");
    }

    const SecureBuffer& dek = m_session_mgr->get_dek(token);

    std::optional<Credential> existing = m_db->get_credential_by_id(credential_id, *uid);
    if (!existing.has_value()) {
        throw VaultException("Credential not found for update");
    }

    EncryptedPayload enc_password = CryptoManager::encrypt_string(
        plaintext_password, dek, "CRED_PASSWORD:" + credential_id
    );

    EncryptedPayload enc_notes;
    if (!plaintext_notes.empty()) {
        enc_notes = CryptoManager::encrypt_string(
            plaintext_notes, dek, "CRED_NOTES:" + credential_id
        );
    }

    Credential updated(
        credential_id,
        *uid,
        title,
        website_url,
        username,
        enc_password,
        enc_notes,
        category,
        favorite,
        existing->created_at(),
        std::chrono::system_clock::now()
    );

    m_db->update_credential(updated);

    m_ledger->record_event(
        *uid,
        AuditEventType::CredentialUpdated,
        credential_id,
        "Updated credential details for " + title
    );
}

bool CredentialManager::delete_credential(const std::string& token, const std::string& credential_id) {
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    if (!uid.has_value()) {
        throw AuthException("Unauthorized session for credential deletion");
    }

    std::optional<Credential> existing = m_db->get_credential_by_id(credential_id, *uid);
    std::string title = existing.has_value() ? existing->title() : "Unknown";

    bool success = m_db->delete_credential(credential_id, *uid);
    if (success) {
        m_ledger->record_event(
            *uid,
            AuditEventType::CredentialDeleted,
            credential_id,
            "Deleted credential record for " + title
        );
    }
    return success;
}

void CredentialManager::toggle_favorite(const std::string& token, const std::string& credential_id) {
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    if (!uid.has_value()) {
        throw AuthException("Unauthorized session for updating favorite");
    }

    std::optional<Credential> existing = m_db->get_credential_by_id(credential_id, *uid);
    if (!existing.has_value()) {
        throw VaultException("Credential not found");
    }

    existing->set_favorite(!existing->is_favorite());
    m_db->update_credential(*existing);
}

} // namespace blockvault

