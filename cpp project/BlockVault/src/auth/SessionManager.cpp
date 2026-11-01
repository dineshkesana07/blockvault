#include "auth/SessionManager.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"

namespace blockvault {

SessionManager::SessionManager(
    std::shared_ptr<DatabaseManager> db,
    int64_t inactivity_timeout_seconds)
    : m_db(std::move(db)),
      m_inactivity_timeout_seconds(inactivity_timeout_seconds) 
{
    if (!m_db) {
        throw AuthException("DatabaseManager pointer cannot be null for SessionManager");
    }
}

std::string SessionManager::create_session(
    const std::string& user_id,
    const std::string& client_ip,
    int64_t duration_seconds) 
{
    std::string token = CryptoManager::generate_random_hex(SESSION_TOKEN_BYTES);
    auto now = std::chrono::system_clock::now();
    auto expires = now + std::chrono::seconds(duration_seconds);
    auto expires_sec = std::chrono::duration_cast<std::chrono::seconds>(expires.time_since_epoch()).count();

    ActiveSession session;
    session.token = token;
    session.user_id = user_id;
    session.client_ip = client_ip;
    session.created_at = now;
    session.last_active = now;
    session.expires_at = expires;
    session.is_unlocked = false;

    m_active_sessions[token] = std::move(session);
    m_db->insert_session(token, user_id, client_ip, expires_sec);

    return token;
}

std::optional<std::string> SessionManager::validate_session(const std::string& token) {
    purge_inactive_sessions();

    auto it = m_active_sessions.find(token);
    if (it != m_active_sessions.end()) {
        auto now = std::chrono::system_clock::now();
        // Check hard expiration
        if (now > it->second.expires_at) {
            terminate_session(token);
            return std::nullopt;
        }

        // Check inactivity timeout
        auto inactive_duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.last_active
        ).count();

        if (inactive_duration > m_inactivity_timeout_seconds) {
            // Lock the vault due to inactivity
            it->second.active_dek.clear();
            it->second.is_unlocked = false;
        }

        // Update last activity
        it->second.last_active = now;
        return it->second.user_id;
    }

    // Fallback: check database session table
    std::optional<std::string> db_uid = m_db->validate_session(token);
    if (db_uid.has_value()) {
        auto now = std::chrono::system_clock::now();
        ActiveSession session;
        session.token = token;
        session.user_id = *db_uid;
        session.client_ip = "RESTORED";
        session.created_at = now;
        session.last_active = now;
        session.expires_at = now + std::chrono::hours(12);
        session.is_unlocked = false;
        m_active_sessions[token] = std::move(session);
        return db_uid;
    }

    return std::nullopt;
}

void SessionManager::unlock_vault(const std::string& token, SecureBuffer dek) {
    auto it = m_active_sessions.find(token);
    if (it == m_active_sessions.end()) {
        throw AuthException("Cannot unlock vault: invalid or expired session token");
    }
    it->second.active_dek = std::move(dek);
    it->second.is_unlocked = true;
    it->second.last_active = std::chrono::system_clock::now();
}

void SessionManager::lock_vault(const std::string& token) {
    auto it = m_active_sessions.find(token);
    if (it != m_active_sessions.end()) {
        it->second.active_dek.clear();
        it->second.is_unlocked = false;
    }
}

bool SessionManager::is_vault_unlocked(const std::string& token) {
    auto it = m_active_sessions.find(token);
    if (it != m_active_sessions.end()) {
        auto now = std::chrono::system_clock::now();
        auto inactive_sec = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.last_active
        ).count();

        if (inactive_sec > m_inactivity_timeout_seconds) {
            it->second.active_dek.clear();
            it->second.is_unlocked = false;
            return false;
        }
        return it->second.is_unlocked;
    }
    return false;
}

const SecureBuffer& SessionManager::get_dek(const std::string& token) {
    auto it = m_active_sessions.find(token);
    if (it == m_active_sessions.end() || !it->second.is_unlocked) {
        throw AuthException("Vault is locked or session is invalid. Master password unlock required.");
    }
    it->second.last_active = std::chrono::system_clock::now();
    return it->second.active_dek;
}

void SessionManager::terminate_session(const std::string& token) {
    auto it = m_active_sessions.find(token);
    if (it != m_active_sessions.end()) {
        it->second.active_dek.clear();
        m_active_sessions.erase(it);
    }
    m_db->delete_session(token);
}

void SessionManager::terminate_all_user_sessions(const std::string& user_id) {
    for (auto it = m_active_sessions.begin(); it != m_active_sessions.end();) {
        if (it->second.user_id == user_id) {
            it->second.active_dek.clear();
            it = m_active_sessions.erase(it);
        } else {
            ++it;
        }
    }
    m_db->delete_user_sessions(user_id);
}

void SessionManager::purge_inactive_sessions() {
    auto now = std::chrono::system_clock::now();
    for (auto it = m_active_sessions.begin(); it != m_active_sessions.end();) {
        if (now > it->second.expires_at) {
            it->second.active_dek.clear();
            m_db->delete_session(it->first);
            it = m_active_sessions.erase(it);
        } else {
            ++it;
        }
    }
    m_db->purge_expired_sessions();
}

} // namespace blockvault

