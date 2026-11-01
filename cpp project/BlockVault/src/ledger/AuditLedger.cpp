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
        result.error_message = "Genesis block hash recalculation failed.";
        return result;
    }

    // 2. Verify all subsequent blocks in sequential order
    for (size_t i = 1; i < blocks.size(); ++i) {
        const Block& prev = blocks[i - 1];
        const Block& curr = blocks[i];

        // Sequence check
        if (curr.index() != prev.index() + 1) {
            result.is_valid = false;
            result.tampered_index = curr.index();
            result.error_message = "Block sequence broken at index " + std::to_string(curr.index());
            return result;
        }

        // Cryptographic link check: curr.previous_hash must match prev.current_hash
        if (!CryptoManager::constant_time_equals(curr.previous_hash(), prev.current_hash())) {
            result.is_valid = false;
            result.tampered_index = curr.index();
            result.error_message = "Hash chain broken between block " + std::to_string(prev.index()) + 
                                   " and block " + std::to_string(curr.index());
            return result;
        }

        // Internal integrity check: recompute hash of this block's contents
        if (!curr.is_valid()) {
            result.is_valid = false;
            result.tampered_index = curr.index();
            result.error_message = "Data tampering detected in block " + std::to_string(curr.index()) + 
                                   ": recorded hash does not match recomputed SHA-256.";
            return result;
        }
    }

    result.is_valid = true;
    result.error_message = "All " + std::to_string(result.total_blocks) + " blocks verified successfully.";
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

