#pragma once

#include "common/Types.hpp"
#include <string>
#include <chrono>

namespace blockvault {

/**
 * @class Credential
 * @brief Domain entity representing an encrypted credential record in the vault.
 * 
 * Stores metadata (title, URL, username, category) and cryptographically authenticated
 * ciphertext blobs for the password and sensitive notes.
 */
class Credential {
private:
    std::string        m_id;                  // UUID v4
    std::string        m_user_id;             // Owning user's UUID
    std::string        m_title;               // Service / website label
    std::string        m_website_url;         // Canonical URL
    std::string        m_username;            // Login handle or email
    EncryptedPayload   m_encrypted_password;  // AEAD encrypted password
    EncryptedPayload   m_encrypted_notes;     // AEAD encrypted secure notes
    CredentialCategory m_category;            // Category classification
    bool               m_favorite{false};     // Pinned to favorites
    Timestamp          m_created_at;          // UTC creation timestamp
    Timestamp          m_updated_at;          // UTC last updated timestamp

public:
    Credential();
    Credential(
        std::string id,
        std::string user_id,
        std::string title,
        std::string website_url,
        std::string username,
        EncryptedPayload encrypted_password,
        EncryptedPayload encrypted_notes = EncryptedPayload{},
        CredentialCategory category = CredentialCategory::Login,
        bool favorite = false,
        Timestamp created_at = std::chrono::system_clock::now(),
        Timestamp updated_at = std::chrono::system_clock::now()
    );

    ~Credential() = default;

    // Value semantics
    Credential(const Credential&) = default;
    Credential& operator=(const Credential&) = default;
    Credential(Credential&&) noexcept = default;
    Credential& operator=(Credential&&) noexcept = default;

    // Getters
    [[nodiscard]] const std::string& id() const noexcept { return m_id; }
    [[nodiscard]] const std::string& user_id() const noexcept { return m_user_id; }
    [[nodiscard]] const std::string& title() const noexcept { return m_title; }
    [[nodiscard]] const std::string& website_url() const noexcept { return m_website_url; }
    [[nodiscard]] const std::string& username() const noexcept { return m_username; }
    [[nodiscard]] const EncryptedPayload& encrypted_password() const noexcept { return m_encrypted_password; }
    [[nodiscard]] const EncryptedPayload& encrypted_notes() const noexcept { return m_encrypted_notes; }
    [[nodiscard]] CredentialCategory category() const noexcept { return m_category; }
    [[nodiscard]] bool is_favorite() const noexcept { return m_favorite; }
    [[nodiscard]] Timestamp created_at() const noexcept { return m_created_at; }
    [[nodiscard]] Timestamp updated_at() const noexcept { return m_updated_at; }

    // Setters
    void set_title(const std::string& title) { m_title = title; touch(); }
    void set_website_url(const std::string& url) { m_website_url = url; touch(); }
    void set_username(const std::string& user) { m_username = user; touch(); }
    void set_encrypted_password(const EncryptedPayload& payload) { m_encrypted_password = payload; touch(); }
    void set_encrypted_notes(const EncryptedPayload& notes) { m_encrypted_notes = notes; touch(); }
    void set_category(CredentialCategory category) { m_category = category; touch(); }
    void set_favorite(bool fav) { m_favorite = fav; touch(); }

    void touch() noexcept { m_updated_at = std::chrono::system_clock::now(); }
};

} // namespace blockvault

