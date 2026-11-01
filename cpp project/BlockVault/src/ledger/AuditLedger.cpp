#include "ledger/AuditLedger.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"

namespace blockvault {

AuditLedger::AuditLedger(std::shared_ptr<DatabaseManager> db)
    : m_db(std::move(db)) 
{
    if (!m_db) {
        throw AuditException("DatabaseManager pointer cannot be null for AuditLedger");
    }
    ensure_genesis_block();
}

void AuditLedger::ensure_genesis_block() {
    if (m_db->get_block_count() == 0) {
        Block genesis = Block::create_genesis_block();
        m_db->insert_block(genesis);
    }
}

Block AuditLedger::record_event(
    const std::string& user_id,
    AuditEventType event_type,
    const std::string& entity_id,
    const std::string& details) 
{
    std::optional<Block> last_block = m_db->get_last_block();
    if (!last_block.has_value()) {
        ensure_genesis_block();
        last_block = m_db->get_last_block();
        if (!last_block.has_value()) {
            throw AuditException("Failed to acquire latest audit block");
        }
    }

    uint64_t next_index = last_block->index() + 1;
    std::string prev_hash = last_block->current_hash();

    Block new_block(
        next_index,
        event_type,
        user_id,
        entity_id,
        details,
        prev_hash,
        std::chrono::system_clock::now()
    );

    m_db->insert_block(new_block);
    return new_block;
}

Block AuditLedger::log_vault_created(const std::string& user_id, const std::string& vault_id) {
    return record_event(user_id, AuditEventType::VaultCreated, vault_id, "User vault storage initialized");
}

Block AuditLedger::log_credential_added(const std::string& user_id, const std::string& credential_id, const std::string& label) {
    return record_event(user_id, AuditEventType::CredentialAdded, credential_id, "Added credential: " + label);
}

Block AuditLedger::log_credential_updated(const std::string& user_id, const std::string& credential_id, const std::string& field_name) {
    return record_event(user_id, AuditEventType::CredentialUpdated, credential_id, "Updated field: " + field_name);
}

Block AuditLedger::log_credential_deleted(const std::string& user_id, const std::string& credential_id) {
    return record_event(user_id, AuditEventType::CredentialDeleted, credential_id, "Deleted credential record");
}

Block AuditLedger::log_vault_locked(const std::string& user_id) {
    return record_event(user_id, AuditEventType::VaultLocked, "", "User session ended; memory enclave wiped");
}

IntegrityVerificationResult AuditLedger::verify_integrity() {
    IntegrityVerificationResult result;
    std::vector<Block> blocks = m_db->get_all_blocks();
    result.total_blocks = blocks.size();

    if (blocks.empty()) {
        result.is_valid = false;
        result.error_message = "Audit ledger is empty (missing Genesis block).";
        return result;
    }

    // 1. Verify Genesis block (Index 0)
    const Block& genesis = blocks[0];
    if (genesis.index() != 0) {
        result.is_valid = false;
        result.tampered_index = 0;
        result.error_message = "Genesis block index is not 0.";
        return result;
    }
    if (genesis.previous_hash() != "0000000000000000000000000000000000000000000000000000000000000000") {
        result.is_valid = false;
        result.tampered_index = 0;
        result.error_message = "Genesis block previous hash is corrupted.";
        return result;
    }
    if (!genesis.is_valid()) {
        result.is_valid = false;
        result.tampered_index = 0;
        result.error_message = "Genesis block hash recalculation failed (tampered genesis payload).";
        return result;
    }

    // 2. Verify all subsequent blocks in sequential order
    for (size_t i = 1; i < blocks.size(); ++i) {
        const Block& prev = blocks[i - 1];
        const Block& curr = blocks[i];

        // Sequence check (detect missing/deleted records)
        if (curr.index() != prev.index() + 1) {
            result.is_valid = false;
            result.tampered_index = curr.index();
            result.error_message = "Missing record detected: gap between index " + 
                                   std::to_string(prev.index()) + " and " + std::to_string(curr.index());
            return result;
        }

        // Cryptographic link check: curr.previous_hash must match prev.current_hash
        if (!CryptoManager::constant_time_equals(curr.previous_hash(), prev.current_hash())) {
            result.is_valid = false;
            result.tampered_index = curr.index();
            result.error_message = "Broken hash link: record #" + std::to_string(curr.index()) + 
                                   " previous_hash does not match hash of record #" + std::to_string(prev.index());
            return result;
        }

        // Timestamp regression check (time cannot flow backward)
        auto prev_sec = std::chrono::duration_cast<std::chrono::seconds>(prev.timestamp().time_since_epoch()).count();
        auto curr_sec = std::chrono::duration_cast<std::chrono::seconds>(curr.timestamp().time_since_epoch()).count();
        if (curr_sec < prev_sec) {
            result.is_valid = false;
            result.tampered_index = curr.index();
            result.error_message = "Timestamp regression detected at record #" + std::to_string(curr.index()) +
                                   ": timestamp precedes prior record.";
            return result;
        }

        // Internal integrity check: recompute SHA-256 of canonical payload
        if (!curr.is_valid()) {
            result.is_valid = false;
            result.tampered_index = curr.index();
            result.error_message = "Content tampering detected in record #" + std::to_string(curr.index()) + 
                                   ": payload modified, hash verification failed.";
            return result;
        }
    }

    result.is_valid = true;
    result.error_message = "All " + std::to_string(result.total_blocks) + " records cryptographically verified.";
    return result;
}

std::vector<Block> AuditLedger::get_all_records() {
    return m_db->get_all_blocks();
}

std::vector<Block> AuditLedger::get_user_records(const std::string& user_id) {
    return m_db->get_blocks_for_user(user_id);
}

std::optional<Block> AuditLedger::get_latest_block() {
    return m_db->get_last_block();
}

uint64_t AuditLedger::get_total_records() {
    return m_db->get_block_count();
}

} // namespace blockvault
