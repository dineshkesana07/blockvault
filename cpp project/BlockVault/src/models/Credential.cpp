#include "models/Credential.hpp"

namespace blockvault {

Credential::Credential()
    : m_category(CredentialCategory::Login),
      m_favorite(false),
      m_created_at(std::chrono::system_clock::now()),
      m_updated_at(std::chrono::system_clock::now()) {}

Credential::Credential(
    std::string id,
    std::string user_id,
    std::string title,
    std::string website_url,
    std::string username,
    EncryptedPayload encrypted_password,
    EncryptedPayload encrypted_notes,
    CredentialCategory category,
    bool favorite,
    Timestamp created_at,
    Timestamp updated_at)
    : m_id(std::move(id)),
      m_user_id(std::move(user_id)),
      m_title(std::move(title)),
      m_website_url(std::move(website_url)),
      m_username(std::move(username)),
      m_encrypted_password(std::move(encrypted_password)),
      m_encrypted_notes(std::move(encrypted_notes)),
      m_category(category),
      m_favorite(favorite),
      m_created_at(created_at),
      m_updated_at(updated_at) {}

} // namespace blockvault

