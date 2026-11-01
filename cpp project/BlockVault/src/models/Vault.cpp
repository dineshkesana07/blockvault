#include "models/Vault.hpp"
#include "common/Exceptions.hpp"
#include <algorithm>

namespace blockvault {

Vault::Vault(std::string user_id) 
    : m_user_id(std::move(user_id)) {}

void Vault::add_credential(const Credential& credential) {
    if (m_credentials.find(credential.id()) != m_credentials.end()) {
        throw VaultException("Credential with ID " + credential.id() + " already exists in vault");
    }
    m_credentials[credential.id()] = credential;
}

void Vault::update_credential(const Credential& credential) {
    auto it = m_credentials.find(credential.id());
    if (it == m_credentials.end()) {
        throw VaultException("Credential with ID " + credential.id() + " not found for update");
    }
    it->second = credential;
}

bool Vault::remove_credential(const std::string& credential_id) {
    return m_credentials.erase(credential_id) > 0;
}

std::optional<Credential> Vault::get_credential(const std::string& credential_id) const {
    auto it = m_credentials.find(credential_id);
    if (it != m_credentials.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool Vault::contains(const std::string& credential_id) const noexcept {
    return m_credentials.find(credential_id) != m_credentials.end();
}

std::vector<Credential> Vault::all() const {
    std::vector<Credential> result;
    result.reserve(m_credentials.size());
    for (const auto& [id, cred] : m_credentials) {
        result.push_back(cred);
    }
    return result;
}

std::vector<Credential> Vault::filter_by_category(CredentialCategory category) const {
    std::vector<Credential> result;
    for (const auto& [id, cred] : m_credentials) {
        if (cred.category() == category) {
            result.push_back(cred);
        }
    }
    return result;
}

std::vector<Credential> Vault::search(const std::string& query) const {
    std::vector<Credential> result;
    if (query.empty()) return all();

    std::string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    for (const auto& [id, cred] : m_credentials) {
        std::string title = cred.title();
        std::string user = cred.username();
        std::string url = cred.website_url();

        auto to_lower = [](std::string& s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
        };
        to_lower(title);
        to_lower(user);
        to_lower(url);

        if (title.find(lower_query) != std::string::npos ||
            user.find(lower_query) != std::string::npos ||
            url.find(lower_query) != std::string::npos) {
            result.push_back(cred);
        }
    }
    return result;
}

std::vector<Credential> Vault::favorites() const {
    std::vector<Credential> result;
    for (const auto& [id, cred] : m_credentials) {
        if (cred.is_favorite()) {
            result.push_back(cred);
        }
    }
    return result;
}

} // namespace blockvault

