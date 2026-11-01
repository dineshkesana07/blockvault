#pragma once

#include "models/Block.hpp"
#include "db/DatabaseManager.hpp"
#include <memory>
#include <vector>
#include <string>

namespace blockvault {

/**
 * @struct IntegrityVerificationResult
 * @brief Detailed report resulting from full audit ledger cryptographic verification.
 */
struct IntegrityVerificationResult {
    bool        is_valid{true};        ///< True if every record is valid, untampered, and linked
    uint64_t    total_blocks{0};       ///< Total number of verified records
    uint64_t    tampered_index{0};     ///< First index where tampering or broken link was detected
    std::string error_message;         ///< Diagnostic explanation of integrity status
};

/**
 * @class AuditLedger
 * @brief Manages the append-only, SHA-256 hash-chained local tamper-evident audit journal.
 * 
 * IMPORTANT ARCHITECTURAL NOTE:
 * This is a local tamper-evident cryptographic hash chain, NOT a decentralized blockchain.
 * It provides mathematical proof of local append-only integrity within SQLite.
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

    // =========================================================================
    // Core Event Recording
    // =========================================================================

    /**
     * @brief Records a security event into the hash chain.
     * Guaranteed never to log passwords, master keys, or plaintexts.
     */
    Block record_event(
        const std::string& user_id,
        AuditEventType event_type,
        const std::string& entity_id = "",
        const std::string& details = ""
    );

    // High-level semantic event helpers
    Block log_vault_created(const std::string& user_id, const std::string& vault_id);
    Block log_credential_added(const std::string& user_id, const std::string& credential_id, const std::string& label);
    Block log_credential_updated(const std::string& user_id, const std::string& credential_id, const std::string& field_name);
    Block log_credential_deleted(const std::string& user_id, const std::string& credential_id);
    Block log_vault_locked(const std::string& user_id);

    // =========================================================================
    // Cryptographic Verification
    // =========================================================================

    /**
     * @brief Cryptographically verifies the integrity of the entire chain.
     * Detects:
     * - Modified record content or metadata
     * - Deleted records (index gaps)
     * - Broken hash links (previous_hash != prior current_hash)
     * - Timestamp regressions
     */
    [[nodiscard]] IntegrityVerificationResult verify_integrity();

    // =========================================================================
    // Queries
    // =========================================================================
    [[nodiscard]] std::vector<Block> get_all_records();
    [[nodiscard]] std::vector<Block> get_user_records(const std::string& user_id);
    [[nodiscard]] std::optional<Block> get_latest_block();
    [[nodiscard]] uint64_t get_total_records();
};

} // namespace blockvault
