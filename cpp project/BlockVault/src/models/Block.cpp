#include "models/Block.hpp"
#include "crypto/CryptoManager.hpp"
#include <sstream>

namespace blockvault {

Block::Block()
    : m_index(0),
      m_timestamp(std::chrono::system_clock::now()),
      m_event_type(AuditEventType::Genesis) {}

Block::Block(
    uint64_t index,
    AuditEventType event_type,
    std::string user_id,
    std::string entity_id,
    std::string details,
    std::string previous_hash,
    Timestamp timestamp)
    : m_index(index),
      m_timestamp(timestamp),
      m_event_type(event_type),
      m_user_id(std::move(user_id)),
      m_entity_id(std::move(entity_id)),
      m_details(std::move(details)),
      m_previous_hash(std::move(previous_hash)) 
{
    m_current_hash = compute_hash();
}

Block::Block(
    uint64_t index,
    AuditEventType event_type,
    std::string user_id,
    std::string entity_id,
    std::string details,
    std::string previous_hash,
    std::string current_hash,
    Timestamp timestamp)
    : m_index(index),
      m_timestamp(timestamp),
      m_event_type(event_type),
      m_user_id(std::move(user_id)),
      m_entity_id(std::move(entity_id)),
      m_details(std::move(details)),
      m_previous_hash(std::move(previous_hash)),
      m_current_hash(std::move(current_hash)) {}

std::string Block::canonical_payload() const {
    auto epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        m_timestamp.time_since_epoch()
    ).count();

    std::ostringstream ss;
    ss << m_index << ":"
       << epoch_seconds << ":"
       << event_type_to_string(m_event_type) << ":"
       << m_user_id << ":"
       << m_entity_id << ":"
       << m_details << ":"
       << m_previous_hash;
    return ss.str();
}

std::string Block::compute_hash() const {
    return CryptoManager::sha256_hex(canonical_payload());
}

bool Block::is_valid() const {
    return CryptoManager::constant_time_equals(m_current_hash, compute_hash());
}

Block Block::create_genesis_block() {
    return Block(
        0,
        AuditEventType::Genesis,
        "SYSTEM",
        "00000000-0000-0000-0000-000000000000",
        "BlockVault Local Cryptographic Audit Ledger Initialized",
        "0000000000000000000000000000000000000000000000000000000000000000"
    );
}

} // namespace blockvault

