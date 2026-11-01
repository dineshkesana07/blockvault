#pragma once

#include "common/Types.hpp"
#include <string>
#include <chrono>

namespace blockvault {

/**
 * @class Block
 * @brief An immutable node in the local SHA-256 hash-chained audit ledger.
 * 
 * Each block records a critical security event and cryptographically incorporates
 * the hash of its predecessor, creating a tamper-evident audit journal.
 */
class Block {
private:
    uint64_t       m_index{0};                  // Sequence position (0 = Genesis)
    Timestamp      m_timestamp;                 // UTC timestamp
    AuditEventType m_event_type;                // Security event category
    std::string    m_user_id;                   // Actor user ID
    std::string    m_entity_id;                 // Target entity identifier
    std::string    m_details;                   // Audit description (sanitized, NO secrets)
    std::string    m_previous_hash;             // 64-character hex hash of block (n-1)
    std::string    m_current_hash;              // 64-character hex hash of this block

public:
    Block();
    Block(
        uint64_t index,
        AuditEventType event_type,
        std::string user_id,
        std::string entity_id,
        std::string details,
        std::string previous_hash,
        Timestamp timestamp = std::chrono::system_clock::now()
    );

    Block(
        uint64_t index,
        AuditEventType event_type,
        std::string user_id,
        std::string entity_id,
        std::string details,
        std::string previous_hash,
        std::string current_hash,
        Timestamp timestamp
    );

    ~Block() = default;

    // Value semantics
    Block(const Block&) = default;
    Block& operator=(const Block&) = default;
    Block(Block&&) noexcept = default;
    Block& operator=(Block&&) noexcept = default;

    // Canonical payload string used for hash calculation
    [[nodiscard]] std::string canonical_payload() const;

    // Computes the SHA-256 hash of the canonical payload
    [[nodiscard]] std::string compute_hash() const;

    // Validates whether m_current_hash matches compute_hash()
    [[nodiscard]] bool is_valid() const;

    // Observers
    [[nodiscard]] uint64_t index() const noexcept { return m_index; }
    [[nodiscard]] Timestamp timestamp() const noexcept { return m_timestamp; }
    [[nodiscard]] AuditEventType event_type() const noexcept { return m_event_type; }
    [[nodiscard]] const std::string& user_id() const noexcept { return m_user_id; }
    [[nodiscard]] const std::string& entity_id() const noexcept { return m_entity_id; }
    [[nodiscard]] const std::string& details() const noexcept { return m_details; }
    [[nodiscard]] const std::string& previous_hash() const noexcept { return m_previous_hash; }
    [[nodiscard]] const std::string& current_hash() const noexcept { return m_current_hash; }

    // Factory method for Genesis Block (Index 0)
    [[nodiscard]] static Block create_genesis_block();
};

} // namespace blockvault

