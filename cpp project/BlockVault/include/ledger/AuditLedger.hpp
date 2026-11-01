#pragma once

#include "models/Block.hpp"
#include "db/DatabaseManager.hpp"
#include <memory>
#include <vector>
#include <string>

namespace blockvault {

struct IntegrityVerificationResult {
    bool        is_valid{true};
    uint64_t    total_blocks{0};
    uint64_t    tampered_index{0};
    std::string error_message;
};

/**
 * @class AuditLedger
 * @brief Manages the append-only, SHA-256 hash-chained tamper-evident audit journal.
 * 
 * Records all security-critical transactions and verifies cryptographic chain integrity.
 */
class AuditLedger {
private:
    std::shared_ptr<DatabaseManager> m_db;

    void ensure_genesis_block();

public:
    explicit AuditLedger(std::shared_ptr<DatabaseManager> db);
    ~AuditLedger() = default;

    // Non-copyable
    AuditLedger(const AuditLedger&) = delete;
    AuditLedger& operator=(const AuditLedger&) = delete;

    // Moveable
    AuditLedger(AuditLedger&&) noexcept = default;
    AuditLedger& operator=(AuditLedger&&) noexcept = default;

    // Record a security event into the hash chain
    Block record_event(
        const std::string& user_id,
        AuditEventType event_type,
        const std::string& entity_id = "",
        const std::string& details = ""
    );

    // Cryptographically verify the integrity of the entire chain
    [[nodiscard]] IntegrityVerificationResult verify_integrity();

    // Query audit records
    [[nodiscard]] std::vector<Block> get_all_records();
    [[nodiscard]] std::vector<Block> get_user_records(const std::string& user_id);
    [[nodiscard]] std::optional<Block> get_latest_block();
    [[nodiscard]] uint64_t get_total_records();
};

} // namespace blockvault

