#pragma once

#include "common/Types.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace blockvault {

/**
 * @struct UserEntity
 * @brief Direct relational mapping of the 'users' SQLite table.
 */
struct UserEntity {
    std::string user_id;    // TEXT PRIMARY KEY
    std::string username;   // TEXT UNIQUE NOT NULL
    ByteVector  salt;       // BLOB NOT NULL (Argon2id salt)
    ByteVector  verifier;   // BLOB NOT NULL (Cryptographic derivation verifier / encrypted DEK)
    int64_t     created_at; // INTEGER NOT NULL (Epoch seconds)
};

/**
 * @struct CredentialEntity
 * @brief Direct relational mapping of the 'credentials' SQLite table.
 */
struct CredentialEntity {
    std::string credential_id;       // TEXT PRIMARY KEY
    std::string user_id;             // TEXT NOT NULL (FOREIGN KEY)
    ByteVector  encrypted_website;   // BLOB NOT NULL (Ciphertext + tag)
    ByteVector  encrypted_username;  // BLOB NOT NULL (Ciphertext + tag)
    ByteVector  encrypted_password;  // BLOB NOT NULL (Ciphertext + tag)
    ByteVector  nonce;               // BLOB NOT NULL (24-byte XChaCha20 nonce)
    std::string category;            // TEXT NOT NULL ('LOGIN', 'SERVER', etc.)
    int64_t     created_at;          // INTEGER NOT NULL (Epoch seconds)
    int64_t     updated_at;          // INTEGER NOT NULL (Epoch seconds)
};

/**
 * @struct AuditRecordEntity
 * @brief Direct relational mapping of the 'audit_records' SQLite table.
 */
struct AuditRecordEntity {
    int64_t     record_id{0};        // INTEGER PRIMARY KEY AUTOINCREMENT
    std::string event_type;          // TEXT NOT NULL
    int64_t     timestamp{0};        // INTEGER NOT NULL (Epoch seconds)
    std::string event_metadata;      // TEXT NOT NULL (JSON/string metadata, NO secrets)
    std::string previous_hash;       // TEXT NOT NULL (SHA-256 hex string)
    std::string current_hash;        // TEXT NOT NULL (SHA-256 hex string)
};

} // namespace blockvault

