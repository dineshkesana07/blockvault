#pragma once

#include "models/Credential.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace blockvault {

/**
 * @class Vault
 * @brief Domain container managing the user's credential records in memory.
 * 
 * Provides search, category filtering, favorites lookup, and collection-level statistics.
 */
class Vault {
private:
    std::string m_user_id;
    std::unordered_map<std::string, Credential> m_credentials;

public:
    explicit Vault(std::string user_id = "");
    ~Vault() = default;

    // Move-only or copyable value semantics
    Vault(const Vault&) = default;
    Vault& operator=(const Vault&) = default;
    Vault(Vault&&) noexcept = default;
    Vault& operator=(Vault&&) noexcept = default;

    [[nodiscard]] const std::string& user_id() const noexcept { return m_user_id; }
    void set_user_id(const std::string& uid) { m_user_id = uid; }

    // CRUD collection operations
    void add_credential(const Credential& credential);
    void update_credential(const Credential& credential);
    bool remove_credential(const std::string& credential_id);
    [[nodiscard]] std::optional<Credential> get_credential(const std::string& credential_id) const;
    [[nodiscard]] bool contains(const std::string& credential_id) const noexcept;

    // Queries
    [[nodiscard]] std::vector<Credential> all() const;
    [[nodiscard]] std::vector<Credential> filter_by_category(CredentialCategory category) const;
    [[nodiscard]] std::vector<Credential> search(const std::string& query) const;
    [[nodiscard]] std::vector<Credential> favorites() const;

    [[nodiscard]] size_t size() const noexcept { return m_credentials.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_credentials.empty(); }
    void clear() noexcept { m_credentials.clear(); }
};

} // namespace blockvault

