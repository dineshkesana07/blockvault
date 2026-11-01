#include "auth/AuthManager.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"

namespace blockvault {

AuthManager::AuthManager(
    std::shared_ptr<DatabaseManager> db,
    std::shared_ptr<SessionManager>  session_mgr,
    std::shared_ptr<AuditLedger>     ledger)
    : m_db(std::move(db)),
      m_session_mgr(std::move(session_mgr)),
      m_ledger(std::move(ledger)) 
{
    if (!m_db || !m_session_mgr || !m_ledger) {
        throw AuthException("Dependencies cannot be null for AuthManager");
    }
}

User AuthManager::register_user(const std::string& email, const std::string& master_password) {
    if (email.empty() || master_password.empty()) {
        throw AuthException("Email and Master Password are required for registration");
    }

    if (m_db->get_user_by_email(email).has_value()) {
        throw AuthException("User with email '" + email + "' already exists");
    }

    // 1. Generate unique random 16-byte cryptographic salt
    ByteVector salt = CryptoManager::generate_random_bytes(ARGON2_SALT_BYTES);

    // 2. Derive Key Encryption Key (KEK) using memory-hard KDF
    SecureBuffer kek = CryptoManager::derive_key(master_password, salt);

    // 3. Compute verification hash: H(KEK)
    std::string password_hash = CryptoManager::sha256_hex(kek.to_string());

    // 4. Generate random 256-bit Data Encryption Key (DEK)
    ByteVector raw_dek = CryptoManager::generate_random_bytes(KEY_BYTES);
    SecureBuffer dek(raw_dek);
    // Securely clear raw temporary buffer
    std::fill(raw_dek.begin(), raw_dek.end(), 0);

    // 5. Wrap DEK with KEK using Authenticated Encryption (AEAD)
    EncryptedPayload encrypted_dek = CryptoManager::encrypt(dek, kek, "USER_DEK_ENVELOPE");

    // 6. Create and persist User record
    std::string user_id = CryptoManager::generate_uuid_v4();
    User new_user(user_id, email, password_hash, salt, encrypted_dek);
    m_db->insert_user(new_user);

    // 7. Record immutable audit block
    m_ledger->record_event(
        user_id,
        AuditEventType::UserRegistered,
        user_id,
        "User account created with zero-knowledge envelope encryption"
    );

    return new_user;
}

std::string AuthManager::login(
    const std::string& email,
    const std::string& master_password,
    const std::string& client_ip) 
{
    std::optional<User> user_opt = m_db->get_user_by_email(email);
    if (!user_opt.has_value()) {
        m_ledger->record_event("UNKNOWN", AuditEventType::UserLoginFailed, "", "Failed login attempt for " + email);
        throw AuthException("Invalid email or master password");
    }

    const User& user = *user_opt;

    // 1. Derive candidate KEK
    SecureBuffer candidate_kek = CryptoManager::derive_key(master_password, user.salt());

    // 2. Compute candidate hash and verify in constant time
    std::string candidate_hash = CryptoManager::sha256_hex(candidate_kek.to_string());
    if (!CryptoManager::constant_time_equals(candidate_hash, user.password_hash())) {
        m_ledger->record_event(user.id(), AuditEventType::UserLoginFailed, user.id(), "Password verification mismatch");
        throw AuthException("Invalid email or master password");
    }

    // 3. Unwrap DEK using KEK
    SecureBuffer unwrapped_dek;
    try {
        unwrapped_dek = CryptoManager::decrypt(user.encrypted_dek(), candidate_kek, "USER_DEK_ENVELOPE");
    } catch (const CryptoException& e) {
        m_ledger->record_event(user.id(), AuditEventType::UserLoginFailed, user.id(), "DEK unwrap failed");
        throw AuthException("Cryptographic verification failed while unwrapping vault keys");
    }

    // 4. Create session and unlock memory enclave with DEK
    std::string token = m_session_mgr->create_session(user.id(), client_ip);
    m_session_mgr->unlock_vault(token, std::move(unwrapped_dek));

    // 5. Audit log
    m_ledger->record_event(user.id(), AuditEventType::UserLoginSuccess, user.id(), "Successful authentication from " + client_ip);

    return token;
}

void AuthManager::unlock_vault(const std::string& token, const std::string& master_password) {
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    if (!uid.has_value()) {
        throw AuthException("Invalid or expired session token");
    }

    std::optional<User> user = m_db->get_user_by_id(*uid);
    if (!user.has_value()) {
        throw AuthException("User record not found");
    }

    SecureBuffer candidate_kek = CryptoManager::derive_key(master_password, user->salt());
    std::string candidate_hash = CryptoManager::sha256_hex(candidate_kek.to_string());
    if (!CryptoManager::constant_time_equals(candidate_hash, user->password_hash())) {
        m_ledger->record_event(*uid, AuditEventType::UserLoginFailed, *uid, "Vault unlock attempt failed");
        throw AuthException("Incorrect master password");
    }

    SecureBuffer unwrapped_dek = CryptoManager::decrypt(user->encrypted_dek(), candidate_kek, "USER_DEK_ENVELOPE");
    m_session_mgr->unlock_vault(token, std::move(unwrapped_dek));

    m_ledger->record_event(*uid, AuditEventType::VaultUnlocked, *uid, "Vault unlocked successfully");
}

void AuthManager::lock_vault(const std::string& token) {
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    m_session_mgr->lock_vault(token);
    if (uid.has_value()) {
        m_ledger->record_event(*uid, AuditEventType::VaultLocked, *uid, "Vault manually locked by user");
    }
}

void AuthManager::logout(const std::string& token) {
    m_session_mgr->terminate_session(token);
}

User AuthManager::get_authenticated_user(const std::string& token) {
    std::optional<std::string> uid = m_session_mgr->validate_session(token);
    if (!uid.has_value()) {
        throw AuthException("Authentication required. Invalid or expired token.");
    }
    std::optional<User> user = m_db->get_user_by_id(*uid);
    if (!user.has_value()) {
        throw AuthException("User profile not found");
    }
    return *user;
}

} // namespace blockvault

