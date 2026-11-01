#include "ledger/AuditLedger.hpp"
#include "db/DatabaseManager.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <memory>

using namespace blockvault;

// ANSI color helpers
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define RUN_TEST(test_func) \
    do { \
        std::cout << "[AUDIT TEST] " << #test_func << " ... "; \
        try { \
            test_func(); \
            std::cout << COLOR_GREEN << "PASSED" << COLOR_RESET << "\n"; \
            g_tests_passed++; \
        } catch (const std::exception& e) { \
            std::cout << COLOR_RED << "FAILED (" << e.what() << ")" << COLOR_RESET << "\n"; \
            g_tests_failed++; \
        } catch (...) { \
            std::cout << COLOR_RED << "FAILED (Unknown Exception)" << COLOR_RESET << "\n"; \
            g_tests_failed++; \
        } \
    } while(0)

// Helper: Remove database files (call AFTER DatabaseManager is destroyed)
static void cleanup_test_db(const std::string& db_path) {
    // On Windows, SQLite WAL mode may leave -wal and -shm files
    std::filesystem::remove(db_path);
    std::filesystem::remove(db_path + "-wal");
    std::filesystem::remove(db_path + "-shm");
}

// =============================================================================
// INDEX MAPPING NOTE:
//
// SQLite record_id is 1-based (auto-increment).
// Block::index() is 0-based: index = record_id - 1.
//
// After AuditLedger construction (auto Genesis):
//   record_id=1  →  block index 0  (Genesis)
//   record_id=2  →  block index 1  (first event)
//   record_id=3  →  block index 2  (second event)
//   ...
//
// SQL tamper queries use record_id; assertions use block index.
// =============================================================================

// =============================================================================
// Test 1: Verify an Untouched Legitimate Ledger
// =============================================================================
void test_untouched_ledger_verification() {
    CryptoManager::initialize();
    std::string db_file = "audit_untouched_test.db";
    cleanup_test_db(db_file);  // Pre-clean

    {
        auto db = std::make_shared<DatabaseManager>(db_file);
        AuditLedger ledger(db);

        // Initial state: Genesis block created automatically
        assert(ledger.get_total_records() == 1);

        // Record sequential lifecycle events
        ledger.log_vault_created("user_alice_uuid", "vault_personal_uuid");
        ledger.log_credential_added("user_alice_uuid", "cred_github_uuid", "GitHub Enterprise");
        ledger.log_credential_updated("user_alice_uuid", "cred_github_uuid", "password");
        ledger.log_credential_deleted("user_alice_uuid", "cred_github_uuid");
        ledger.log_vault_locked("user_alice_uuid");

        assert(ledger.get_total_records() == 6);

        // Verify complete chain
        IntegrityVerificationResult res = ledger.verify_integrity();
        assert(res.is_valid == true);
        assert(res.total_blocks == 6);
        assert(res.tampered_index == 0);
    } // db & ledger destroyed here → SQLite connection closed

    cleanup_test_db(db_file);
}

// =============================================================================
// Test 2: Detect Modified Existing Record Content
// =============================================================================
void test_modify_existing_record() {
    CryptoManager::initialize();
    std::string db_file = "audit_modify_test.db";
    cleanup_test_db(db_file);

    {
        auto db = std::make_shared<DatabaseManager>(db_file);
        AuditLedger ledger(db);

        // record_id=1 → index 0 (Genesis)
        // record_id=2 → index 1 (VaultCreated)
        // record_id=3 → index 2 (CredentialAdded)
        // record_id=4 → index 3 (VaultLocked)
        ledger.log_vault_created("user_bob_uuid", "vault_bob_uuid");
        ledger.log_credential_added("user_bob_uuid", "cred_aws_uuid", "AWS Production");
        ledger.log_vault_locked("user_bob_uuid");

        assert(ledger.verify_integrity().is_valid == true);

        // Attack simulation: Modify record_id=3 (block index 2) event_type
        db->execute_raw("UPDATE audit_records SET event_type = 'VAULT_UNLOCKED' WHERE record_id = 3;");

        // Verification must detect the discrepancy at block index 2
        IntegrityVerificationResult res = ledger.verify_integrity();
        assert(res.is_valid == false);
        assert(res.tampered_index == 2);
        assert(res.error_message.find("Content tampering") != std::string::npos ||
               res.error_message.find("payload modified") != std::string::npos);
    }

    cleanup_test_db(db_file);
}

// =============================================================================
// Test 3: Detect Deleted Record (Missing Index Gap)
// =============================================================================
void test_delete_record() {
    CryptoManager::initialize();
    std::string db_file = "audit_delete_test.db";
    cleanup_test_db(db_file);

    {
        auto db = std::make_shared<DatabaseManager>(db_file);
        AuditLedger ledger(db);

        // record_id=1 → index 0 (Genesis)
        // record_id=2 → index 1 (VaultCreated)
        // record_id=3 → index 2 (CredentialAdded cred_1)
        // record_id=4 → index 3 (CredentialAdded cred_2)
        // record_id=5 → index 4 (VaultLocked)
        ledger.log_vault_created("user_carol_uuid", "vault_carol_uuid");
        ledger.log_credential_added("user_carol_uuid", "cred_1", "Secret Note 1");
        ledger.log_credential_added("user_carol_uuid", "cred_2", "Secret Note 2");
        ledger.log_vault_locked("user_carol_uuid");

        assert(ledger.get_total_records() == 5);
        assert(ledger.verify_integrity().is_valid == true);

        // Attack simulation: Delete record_id=3 (block index 2)
        db->execute_raw("DELETE FROM audit_records WHERE record_id = 3;");

        // After deletion, DB contains record_ids: 1, 2, 4, 5
        // mapped to block indices: 0, 1, 3, 4
        // Gap detected: jump from index 1 to 3 (missing index 2)
        // verify_integrity() reports tampered_index = 3 (the first block after the gap)
        IntegrityVerificationResult res = ledger.verify_integrity();
        assert(res.is_valid == false);
        assert(res.tampered_index == 3);
        assert(res.error_message.find("Missing record") != std::string::npos || 
               res.error_message.find("Broken hash link") != std::string::npos);
    }

    cleanup_test_db(db_file);
}

// =============================================================================
// Test 4: Detect Changed Previous Hash (Broken Chain Link)
// =============================================================================
void test_change_previous_hash() {
    CryptoManager::initialize();
    std::string db_file = "audit_prevhash_test.db";
    cleanup_test_db(db_file);

    {
        auto db = std::make_shared<DatabaseManager>(db_file);
        AuditLedger ledger(db);

        // record_id=1 → index 0 (Genesis)
        // record_id=2 → index 1 (VaultCreated)
        // record_id=3 → index 2 (CredentialAdded)
        // record_id=4 → index 3 (VaultLocked)
        ledger.log_vault_created("user_dave_uuid", "vault_dave_uuid");
        ledger.log_credential_added("user_dave_uuid", "cred_slack", "Slack Token");
        ledger.log_vault_locked("user_dave_uuid");

        assert(ledger.verify_integrity().is_valid == true);

        // Attack simulation: Corrupt previous_hash of record_id=4 (block index 3)
        std::string fake_prev_hash = "deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef";
        db->execute_raw("UPDATE audit_records SET previous_hash = '" + fake_prev_hash + "' WHERE record_id = 4;");

        // Verification must flag the broken hash link at block index 3
        IntegrityVerificationResult res = ledger.verify_integrity();
        assert(res.is_valid == false);
        assert(res.tampered_index == 3);
    }

    cleanup_test_db(db_file);
}

// =============================================================================
// Test 5: Detect Tampered Event Metadata
// =============================================================================
void test_change_event_metadata() {
    CryptoManager::initialize();
    std::string db_file = "audit_meta_test.db";
    cleanup_test_db(db_file);

    {
        auto db = std::make_shared<DatabaseManager>(db_file);
        AuditLedger ledger(db);

        // record_id=1 → index 0 (Genesis)
        // record_id=2 → index 1 (VaultCreated)
        // record_id=3 → index 2 (CredentialAdded "Bank Account")
        // record_id=4 → index 3 (VaultLocked)
        ledger.log_vault_created("user_eve_uuid", "vault_eve_uuid");
        ledger.log_credential_added("user_eve_uuid", "cred_bank", "Bank Account");
        ledger.log_vault_locked("user_eve_uuid");

        assert(ledger.verify_integrity().is_valid == true);

        // Attack simulation: Alter metadata of record_id=3 (block index 2)
        db->execute_raw("UPDATE audit_records SET event_metadata = 'user_eve_uuid:cred_bank:FORGED Transfer' WHERE record_id = 3;");

        // Verification must detect that recomputing SHA-256 fails at block index 2
        IntegrityVerificationResult res = ledger.verify_integrity();
        assert(res.is_valid == false);
        assert(res.tampered_index == 2);
        assert(res.error_message.find("Content tampering") != std::string::npos ||
               res.error_message.find("payload modified") != std::string::npos);
    }

    cleanup_test_db(db_file);
}

// =============================================================================
// Test 6: Detect Full Re-Hash Tampering at Next Block
// =============================================================================
void test_rehash_cascade_detection() {
    // Demonstrates why local hash chaining works:
    // Even if an attacker modifies record #3 AND recalculates its current_hash
    // so that record #3 appears internally valid, record #4's previous_hash still
    // points to the OLD hash, so the chain breaks at record #4!
    CryptoManager::initialize();
    std::string db_file = "audit_rehash_test.db";
    cleanup_test_db(db_file);

    {
        auto db = std::make_shared<DatabaseManager>(db_file);
        AuditLedger ledger(db);

        // record_id=1 → index 0 (Genesis)
        // record_id=2 → index 1 (VaultCreated)
        // record_id=3 → index 2 (CredentialAdded "Original Credential")
        // record_id=4 → index 3 (VaultLocked)
        ledger.log_vault_created("user_frank_uuid", "vault_frank_uuid");
        Block b2 = ledger.log_credential_added("user_frank_uuid", "cred_1", "Original Credential");
        ledger.log_vault_locked("user_frank_uuid");

        assert(b2.index() == 2);  // Sanity check
        assert(ledger.verify_integrity().is_valid == true);

        // Attacker modifies record_id=3 (block index 2) metadata AND recomputes
        // a valid current_hash for that block
        Block forged_b2(
            b2.index(),
            b2.event_type(),
            b2.user_id(),
            b2.entity_id(),
            "Forged Metadata",
            b2.previous_hash(),
            b2.timestamp()
        );

        // The DB stores event_metadata in format: "user_id:entity_id:details"
        std::string forged_meta = forged_b2.user_id() + ":" + forged_b2.entity_id() + ":" + forged_b2.details();
        std::string update_sql = "UPDATE audit_records SET event_metadata = '" + forged_meta +
                                 "', current_hash = '" + forged_b2.current_hash() + 
                                 "' WHERE record_id = 3;";
        db->execute_raw(update_sql);

        // Now block index 2 is internally consistent! But block index 3 still
        // points to the old hash of block index 2 via its previous_hash field!
        IntegrityVerificationResult res = ledger.verify_integrity();
        assert(res.is_valid == false);
        // Chain break caught at block index 3 (the next block after the forged one)
        assert(res.tampered_index == 3);
        assert(res.error_message.find("Broken hash link") != std::string::npos);
    }

    cleanup_test_db(db_file);
}

// =============================================================================
// Main Test Runner Entrypoint
// =============================================================================
int main() {
    std::cout << "\n========================================================\n";
    std::cout << " BLOCKVAULT C++20 AUDIT LEDGER TEST SUITE\n";
    std::cout << "========================================================\n\n";

    RUN_TEST(test_untouched_ledger_verification);
    RUN_TEST(test_modify_existing_record);
    RUN_TEST(test_delete_record);
    RUN_TEST(test_change_previous_hash);
    RUN_TEST(test_change_event_metadata);
    RUN_TEST(test_rehash_cascade_detection);

    std::cout << "\n--------------------------------------------------------\n";
    std::cout << " AUDIT LEDGER RESULTS: " 
              << g_tests_passed << " Passed, " 
              << g_tests_failed << " Failed\n";
    std::cout << "========================================================\n\n";

    return (g_tests_failed == 0) ? 0 : 1;
}
