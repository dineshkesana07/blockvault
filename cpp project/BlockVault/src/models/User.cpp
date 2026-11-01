#include "models/User.hpp"

namespace blockvault {

User::User() 
    : m_created_at(std::chrono::system_clock::now()),
      m_updated_at(std::chrono::system_clock::now()) {}

User::User(
    std::string id,
    std::string email,
    std::string password_hash,
    ByteVector salt,
    EncryptedPayload encrypted_dek,
    Timestamp created_at,
    Timestamp updated_at)
    : m_id(std::move(id)),
      m_email(std::move(email)),
      m_password_hash(std::move(password_hash)),
      m_salt(std::move(salt)),
      m_encrypted_dek(std::move(encrypted_dek)),
      m_created_at(created_at),
      m_updated_at(updated_at) {}

} // namespace blockvault

