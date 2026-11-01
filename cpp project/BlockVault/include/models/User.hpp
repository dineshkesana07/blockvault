#pragma once

#include "common/Types.hpp"
#include <string>
#include <chrono>

namespace blockvault {

/**
 * @class User
 * @brief Domain entity representing an authenticated BlockVault account.
 * 
 * Encapsulates the user's identity, master password verification material (salt and hash),
 * and the encrypted Data Encryption Key (DEK) wrapped under the user's Key Encryption Key (KEK).
 */
class User {
private:
    std::string      m_id;             // UUID v4 identifier
    std::string      m_email;          // Unique user email address
    std::string      m_password_hash;  // Hex-encoded master password derivation verifier
    ByteVector       m_salt;           // 16-byte cryptographic salt
    EncryptedPayload m_encrypted_dek;  // DEK wrapped by KEK
    Timestamp        m_created_at;     // UTC creation time
    Timestamp        m_updated_at;     // UTC last update time

public:
    User();
    User(
        std::string id,
        std::string email,
        std::string password_hash,
        ByteVector salt,
        EncryptedPayload encrypted_dek,
        Timestamp created_at = std::chrono::system_clock::now(),
        Timestamp updated_at = std::chrono::system_clock::now()
    );

    ~User() = default;

    // Standard value semantics
    User(const User&) = default;
    User& operator=(const User&) = default;
    User(User&&) noexcept = default;
    User& operator=(User&&) noexcept = default;

    // Getters
    [[nodiscard]] const std::string& id() const noexcept { return m_id; }
    [[nodiscard]] const std::string& email() const noexcept { return m_email; }
    [[nodiscard]] const std::string& password_hash() const noexcept { return m_password_hash; }
    [[nodiscard]] const ByteVector& salt() const noexcept { return m_salt; }
    [[nodiscard]] const EncryptedPayload& encrypted_dek() const noexcept { return m_encrypted_dek; }
    [[nodiscard]] Timestamp created_at() const noexcept { return m_created_at; }
    [[nodiscard]] Timestamp updated_at() const noexcept { return m_updated_at; }

    // Setters
    void set_email(const std::string& email) { m_email = email; touch(); }
    void set_password_hash(const std::string& hash) { m_password_hash = hash; touch(); }
    void set_salt(const ByteVector& salt) { m_salt = salt; touch(); }
    void set_encrypted_dek(const EncryptedPayload& dek) { m_encrypted_dek = dek; touch(); }

    // Touch updated timestamp
    void touch() noexcept { m_updated_at = std::chrono::system_clock::now(); }
};

} // namespace blockvault

