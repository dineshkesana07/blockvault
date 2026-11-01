#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <optional>

namespace blockvault {

// Strong alias types for cryptographic data buffers
using ByteVector = std::vector<uint8_t>;
using Timestamp = std::chrono::system_clock::time_point;

// Cryptographic constants
constexpr size_t ARGON2_SALT_BYTES = 16;
constexpr size_t KEY_BYTES = 32;          // 256-bit symmetric key
constexpr size_t NONCE_BYTES = 24;        // 192-bit XChaCha20 nonce
constexpr size_t MAC_BYTES = 16;          // 128-bit Poly1305 MAC tag
constexpr size_t SHA256_DIGEST_BYTES = 32;// 256-bit SHA-256 digest
constexpr size_t SESSION_TOKEN_BYTES = 32;// 256-bit random session token

// Credential categories
enum class CredentialCategory {
    Login,
    SecureNote,
    CreditCard,
    Server,
    Identity
};

// Convert CredentialCategory to and from string representation
inline std::string category_to_string(CredentialCategory category) {
    switch (category) {
        case CredentialCategory::Login: return "LOGIN";
        case CredentialCategory::SecureNote: return "SECURE_NOTE";
        case CredentialCategory::CreditCard: return "CREDIT_CARD";
        case CredentialCategory::Server: return "SERVER";
        case CredentialCategory::Identity: return "IDENTITY";
        default: return "LOGIN";
    }
}

inline CredentialCategory string_to_category(const std::string& str) {
    if (str == "SECURE_NOTE") return CredentialCategory::SecureNote;
    if (str == "CREDIT_CARD") return CredentialCategory::CreditCard;
    if (str == "SERVER") return CredentialCategory::Server;
    if (str == "IDENTITY") return CredentialCategory::Identity;
    return CredentialCategory::Login;
}

// Audit event types for the hash-linked ledger
enum class AuditEventType {
    Genesis,
    UserRegistered,
    UserLoginSuccess,
    UserLoginFailed,
    VaultUnlocked,
    VaultLocked,
    CredentialCreated,
    CredentialRead,
    CredentialUpdated,
    CredentialDeleted,
    VaultExported,
    VaultImported,
    IntegrityVerified
};

inline std::string event_type_to_string(AuditEventType type) {
    switch (type) {
        case AuditEventType::Genesis: return "GENESIS";
        case AuditEventType::UserRegistered: return "USER_REGISTERED";
        case AuditEventType::UserLoginSuccess: return "USER_LOGIN_SUCCESS";
        case AuditEventType::UserLoginFailed: return "USER_LOGIN_FAILED";
        case AuditEventType::VaultUnlocked: return "VAULT_UNLOCKED";
        case AuditEventType::VaultLocked: return "VAULT_LOCKED";
        case AuditEventType::CredentialCreated: return "CREDENTIAL_CREATED";
        case AuditEventType::CredentialRead: return "CREDENTIAL_READ";
        case AuditEventType::CredentialUpdated: return "CREDENTIAL_UPDATED";
        case AuditEventType::CredentialDeleted: return "CREDENTIAL_DELETED";
        case AuditEventType::VaultExported: return "VAULT_EXPORTED";
        case AuditEventType::VaultImported: return "VAULT_IMPORTED";
        case AuditEventType::IntegrityVerified: return "INTEGRITY_VERIFIED";
        default: return "UNKNOWN";
    }
}

inline AuditEventType string_to_event_type(const std::string& str) {
    if (str == "GENESIS") return AuditEventType::Genesis;
    if (str == "USER_REGISTERED") return AuditEventType::UserRegistered;
    if (str == "USER_LOGIN_SUCCESS") return AuditEventType::UserLoginSuccess;
    if (str == "USER_LOGIN_FAILED") return AuditEventType::UserLoginFailed;
    if (str == "VAULT_UNLOCKED") return AuditEventType::VaultUnlocked;
    if (str == "VAULT_LOCKED") return AuditEventType::VaultLocked;
    if (str == "CREDENTIAL_CREATED") return AuditEventType::CredentialCreated;
    if (str == "CREDENTIAL_READ") return AuditEventType::CredentialRead;
    if (str == "CREDENTIAL_UPDATED") return AuditEventType::CredentialUpdated;
    if (str == "CREDENTIAL_DELETED") return AuditEventType::CredentialDeleted;
    if (str == "VAULT_EXPORTED") return AuditEventType::VaultExported;
    if (str == "VAULT_IMPORTED") return AuditEventType::VaultImported;
    if (str == "INTEGRITY_VERIFIED") return AuditEventType::IntegrityVerified;
    return AuditEventType::Genesis;
}

// Struct representing an authenticated ciphertext payload
struct EncryptedPayload {
    ByteVector nonce;       // 24 bytes for XChaCha20
    ByteVector ciphertext;  // Encrypted ciphertext
    ByteVector tag;         // 16 bytes Poly1305 MAC tag
};

} // namespace blockvault

