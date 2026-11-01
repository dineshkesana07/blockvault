#include "db/DatabaseManager.hpp"
#include "db/PreparedStatement.hpp"
#include "db/Transaction.hpp"
#include "db/MigrationManager.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"

#include <iostream>
#include <cassert>
#include <vector>
#include <string>

using namespace blockvault;

// ANSI Terminal Colors
#define GREEN "\033[32m"
#define RED   "\033[31m"
#define CYAN  "\033[36m"
#define BOLD  "\033[1m"
#define RESET "\033[0m"

static int g_db_passed = 0;
static int g_db_failed = 0;

#define DB_TEST(name) \
    std::cout << CYAN << "[DB TEST] " << RESET << name << " ... "; \
    try {

#define DB_PASS() \
        std::cout << GREEN << BOLD << "PASSED" << RESET << "\n"; \
        g_db_passed++; \
    } catch (const std::exception& e) { \
        std::cout << RED << BOLD << "FAILED: " << e.what() << RESET << "\n"; \
        g_db_failed++; \
    }

// -----------------------------------------------------------------------------
// Test 1: Database Connection & PRAGMA Enforcement
// -----------------------------------------------------------------------------
void test_connection_and_pragmas() {
    DB_TEST("Database Connection, WAL & Foreign Key Pragmas")
        DatabaseManager db(":memory:");
        assert(db.is_open());

        // Verify foreign keys are enabled (PRAGMA foreign_keys must return 1)
        PreparedStatement stmt_fk(db.handle(), "PRAGMA foreign_keys;");
        assert(stmt_fk.step());
        assert(stmt_fk.get_int(0) == 1);

        // Verify busy timeout pragma
        PreparedStatement stmt_bt(db.handle(), "PRAGMA busy_timeout;");
        assert(stmt_bt.step());
        assert(stmt_bt.get_int(0) == 5000);
    DB_PASS()
}

// -----------------------------------------------------------------------------
// Test 2: Schema Versioning & Migrations Engine
// -----------------------------------------------------------------------------
void test_schema_migrations() {
    DB_TEST("Schema Versioning, Migrations & Idempotency")
        DatabaseManager db(":memory:");
        
        int version = db.get_schema_version();
        assert(version >= 2);

        // Verify migration history table has recorded versions
        auto history = MigrationManager::get_history(db.handle());
        assert(history.size() >= 2);
        assert(history[0].version == 1);
        assert(history[1].version == 2);

        // Idempotency: re-running migrations applies 0 new migrations and throws nothing
        int second_run = MigrationManager::apply_all(db.handle());
        assert(second_run == 0);
        assert(db.get_schema_version() == version);
    DB_PASS()
}

// -----------------------------------------------------------------------------
// Test 3: Users Table CRUD Operations
// -----------------------------------------------------------------------------
void test_users_crud() {
    DB_TEST("Users Table Complete CRUD Operations")
        DatabaseManager db(":memory:");

        // 1. CREATE / INSERT
        ByteVector salt = CryptoManager::generate_random_bytes(16);
        ByteVector verifier = CryptoManager::generate_random_bytes(64); // Argon2id verifier blob

        UserEntity user1{
            .user_id = "usr_001_alpha",
            .username = "sec_admin",
            .salt = salt,
            .verifier = verifier,
            .created_at = 1730000000
        };
        db.insert_user_entity(user1);

        // 2. READ / SELECT by user_id
        auto by_id = db.get_user_entity_by_id("usr_001_alpha");
        assert(by_id.has_value());
        assert(by_id->username == "sec_admin");
        assert(by_id->salt == salt);
        assert(by_id->verifier == verifier);
        assert(by_id->created_at == 1730000000);

        // 3. READ / SELECT by username
        auto by_name = db.get_user_entity_by_username("sec_admin");
        assert(by_name.has_value());
        assert(by_name->user_id == "usr_001_alpha");

        // 4. UPDATE verifier & salt
        ByteVector new_salt = CryptoManager::generate_random_bytes(16);
        ByteVector new_verifier = CryptoManager::generate_random_bytes(64);
        db.update_user_entity_verifier("usr_001_alpha", new_salt, new_verifier);

        auto updated = db.get_user_entity_by_id("usr_001_alpha");
        assert(updated.has_value());
        assert(updated->salt == new_salt);
        assert(updated->verifier == new_verifier);

        // 5. DELETE
        bool deleted = db.delete_user_entity("usr_001_alpha");
        assert(deleted);
        assert(!db.get_user_entity_by_id("usr_001_alpha").has_value());
    DB_PASS()
}

// -----------------------------------------------------------------------------
// Test 4: Credentials Table CRUD with Binary BLOB Storage
// -----------------------------------------------------------------------------
void test_credentials_crud() {
    DB_TEST("Credentials Table CRUD with Binary BLOB Storage")
        DatabaseManager db(":memory:");

        // Create owning user
        UserEntity user{
            .user_id = "usr_100",
            .username = "developer",
            .salt = CryptoManager::generate_random_bytes(16),
            .verifier = CryptoManager::generate_random_bytes(32),
            .created_at = 1730000100
        };
        db.insert_user_entity(user);

        // Encrypted blobs
        ByteVector enc_site = CryptoManager::generate_random_bytes(48);
        ByteVector enc_user = CryptoManager::generate_random_bytes(32);
        ByteVector enc_pass = CryptoManager::generate_random_bytes(64);
        ByteVector nonce = CryptoManager::generate_random_bytes(24);

        // 1. INSERT
        CredentialEntity cred1{
            .credential_id = "cred_001",
            .user_id = "usr_100",
            .encrypted_website = enc_site,
            .encrypted_username = enc_user,
            .encrypted_password = enc_pass,
            .nonce = nonce,
            .category = "SERVER",
            .created_at = 1730000200,
            .updated_at = 1730000200
        };
        db.insert_credential_entity(cred1);

        // 2. SELECT by ID and user_id (Tenant Isolation)
        auto fetched = db.get_credential_entity_by_id("cred_001", "usr_100");
        assert(fetched.has_value());
        assert(fetched->category == "SERVER");
        assert(fetched->encrypted_website == enc_site);
        assert(fetched->encrypted_username == enc_user);
        assert(fetched->encrypted_password == enc_pass);
        assert(fetched->nonce == nonce);

        // Verify another user cannot access this credential (IDOR defense)
        assert(!db.get_credential_entity_by_id("cred_001", "usr_999").has_value());

        // 3. UPDATE
        ByteVector new_enc_pass = CryptoManager::generate_random_bytes(64);
        fetched->encrypted_password = new_enc_pass;
        fetched->category = "LOGIN";
        fetched->updated_at = 1730000300;
        db.update_credential_entity(*fetched);

        auto updated = db.get_credential_entity_by_id("cred_001", "usr_100");
        assert(updated.has_value());
        assert(updated->encrypted_password == new_enc_pass);
        assert(updated->category == "LOGIN");
        assert(updated->updated_at == 1730000300);

        // 4. LIST FOR USER
        auto list = db.get_credential_entities_for_user("usr_100");
        assert(list.size() == 1);

        // 5. DELETE
        bool deleted = db.delete_credential_entity("cred_001", "usr_100");
        assert(deleted);
        assert(db.get_credential_entity_count("usr_100") == 0);
    DB_PASS()
}

// -----------------------------------------------------------------------------
// Test 5: Audit Records Table CRUD
// -----------------------------------------------------------------------------
void test_audit_records_crud() {
    DB_TEST("Audit Records CRUD & Auto-Increment Sequence")
        DatabaseManager db(":memory:");

        AuditRecordEntity r1{
            .event_type = "GENESIS",
            .timestamp = 1730000000,
            .event_metadata = "{\"msg\":\"Genesis block initialized\"}",
            .previous_hash = "0000000000000000000000000000000000000000000000000000000000000000",
            .current_hash = "a1b2c3d4e5f60718293a4b5c6d7e8f901234567890abcdef1234567890abcdef"
        };
        int64_t id1 = db.insert_audit_record(r1);
        assert(id1 == 1);

        AuditRecordEntity r2{
            .event_type = "USER_REGISTERED",
            .timestamp = 1730000010,
            .event_metadata = "{\"user\":\"usr_001\"}",
            .previous_hash = r1.current_hash,
            .current_hash = "b2c3d4e5f60718293a4b5c6d7e8f901234567890abcdef1234567890abcdef12"
        };
        int64_t id2 = db.insert_audit_record(r2);
        assert(id2 == 2);

        // Verify last audit record
        auto last = db.get_last_audit_record();
        assert(last.has_value());
        assert(last->record_id == 2);
        assert(last->event_type == "USER_REGISTERED");
        assert(last->previous_hash == r1.current_hash);

        // Verify total count
        assert(db.get_audit_record_count() == 2);

        // Verify filter by event
        auto genesis_records = db.get_audit_records_by_event("GENESIS");
        assert(genesis_records.size() == 1);
        assert(genesis_records[0].record_id == 1);
    DB_PASS()
}

// -----------------------------------------------------------------------------
// Test 6: Database Failure Scenarios & Edge Cases
// -----------------------------------------------------------------------------
void test_database_failure_scenarios() {
    // Failure Scenario A: Unique Constraint Violation
    DB_TEST("Failure Scenario A: Unique Constraint Violation (Duplicate Username)")
        DatabaseManager db(":memory:");

        UserEntity u1{
            .user_id = "u-001",
            .username = "unique_admin",
            .salt = CryptoManager::generate_random_bytes(16),
            .verifier = CryptoManager::generate_random_bytes(32),
            .created_at = 1730000000
        };
        db.insert_user_entity(u1);

        // Attempt duplicate username with different user_id
        UserEntity u2{
            .user_id = "u-002",
            .username = "unique_admin", // DUPLICATE!
            .salt = CryptoManager::generate_random_bytes(16),
            .verifier = CryptoManager::generate_random_bytes(32),
            .created_at = 1730000005
        };

        bool caught_unique_error = false;
        try {
            db.insert_user_entity(u2);
        } catch (const DatabaseException& e) {
            caught_unique_error = true;
            // Confirm the error specifies UNIQUE constraint failure
            std::string msg = e.what();
            assert(msg.find("UNIQUE") != std::string::npos || msg.find("constraint") != std::string::npos);
        }
        assert(caught_unique_error);
    DB_PASS()

    // Failure Scenario B: Foreign Key Constraint Violation
    DB_TEST("Failure Scenario B: Foreign Key Constraint Violation (Orphan Credential)")
        DatabaseManager db(":memory:");

        CredentialEntity orphan{
            .credential_id = "cred_orphan",
            .user_id = "non_existent_user_id_404", // FK does not exist!
            .encrypted_website = CryptoManager::generate_random_bytes(32),
            .encrypted_username = CryptoManager::generate_random_bytes(32),
            .encrypted_password = CryptoManager::generate_random_bytes(32),
            .nonce = CryptoManager::generate_random_bytes(24),
            .category = "LOGIN",
            .created_at = 1730000000,
            .updated_at = 1730000000
        };

        bool caught_fk_error = false;
        try {
            db.insert_credential_entity(orphan);
        } catch (const DatabaseException& e) {
            caught_fk_error = true;
            std::string msg = e.what();
            assert(msg.find("FOREIGN KEY") != std::string::npos || msg.find("constraint") != std::string::npos);
        }
        assert(caught_fk_error);
    DB_PASS()

    // Failure Scenario C: Cascading Deletion via Foreign Key
    DB_TEST("Failure Scenario C: Foreign Key ON DELETE CASCADE Verification")
        DatabaseManager db(":memory:");

        UserEntity user{
            .user_id = "user_cascade_test",
            .username = "cascader",
            .salt = CryptoManager::generate_random_bytes(16),
            .verifier = CryptoManager::generate_random_bytes(32),
            .created_at = 1730000000
        };
        db.insert_user_entity(user);

        // Add 3 credentials for this user
        for (int i = 1; i <= 3; ++i) {
            CredentialEntity c{
                .credential_id = "c_" + std::to_string(i),
                .user_id = "user_cascade_test",
                .encrypted_website = CryptoManager::generate_random_bytes(16),
                .encrypted_username = CryptoManager::generate_random_bytes(16),
                .encrypted_password = CryptoManager::generate_random_bytes(16),
                .nonce = CryptoManager::generate_random_bytes(24),
                .category = "LOGIN",
                .created_at = 1730000000,
                .updated_at = 1730000000
            };
            db.insert_credential_entity(c);
        }
        assert(db.get_credential_entity_count("user_cascade_test") == 3);

        // Delete user -> credentials must be purged automatically
        db.delete_user_entity("user_cascade_test");
        assert(!db.get_user_entity_by_id("user_cascade_test").has_value());
        assert(db.get_credential_entity_count("user_cascade_test") == 0);
    DB_PASS()

    // Failure Scenario D: Transaction Atomic Rollback on Mid-Operation Failure
    DB_TEST("Failure Scenario D: Transaction Atomic Rollback on Mid-Operation Exception")
        DatabaseManager db(":memory:");

        UserEntity u{
            .user_id = "u_tx",
            .username = "tx_user",
            .salt = CryptoManager::generate_random_bytes(16),
            .verifier = CryptoManager::generate_random_bytes(32),
            .created_at = 1730000000
        };
        db.insert_user_entity(u);

        // Multi-step transaction: step 1 succeeds, step 2 violates FK and throws
        try {
            Transaction tx = db.create_transaction();

            CredentialEntity c_valid{
                .credential_id = "c_valid",
                .user_id = "u_tx",
                .encrypted_website = CryptoManager::generate_random_bytes(16),
                .encrypted_username = CryptoManager::generate_random_bytes(16),
                .encrypted_password = CryptoManager::generate_random_bytes(16),
                .nonce = CryptoManager::generate_random_bytes(24),
                .category = "LOGIN",
                .created_at = 1730000000,
                .updated_at = 1730000000
            };
            db.insert_credential_entity(c_valid);

            // Deliberate constraint violation: non-existent user
            CredentialEntity c_invalid = c_valid;
            c_invalid.credential_id = "c_invalid";
            c_invalid.user_id = "non_existent_foreign_key";
            db.insert_credential_entity(c_invalid); // Throws DatabaseException!

            tx.commit(); // Never reached
        } catch (const DatabaseException&) {
            // Exception caught; tx destructor invoked, rolling back
        }

        // Verify that c_valid was rolled back and DOES NOT exist in the database!
        assert(!db.get_credential_entity_by_id("c_valid", "u_tx").has_value());
        assert(db.get_credential_entity_count("u_tx") == 0);
    DB_PASS()
}

// -----------------------------------------------------------------------------
// Main Test Runner
// -----------------------------------------------------------------------------
int main() {
    std::cout << "\n" << BOLD << "========================================================\n";
    std::cout << " BLOCKVAULT C++20 DATABASE MODULE TEST SUITE\n";
    std::cout << "========================================================" << RESET << "\n\n";

    test_connection_and_pragmas();
    test_schema_migrations();
    test_users_crud();
    test_credentials_crud();
    test_audit_records_crud();
    test_database_failure_scenarios();

    std::cout << "\n" << BOLD << "--------------------------------------------------------\n";
    std::cout << " DATABASE RESULTS: "
              << GREEN << g_db_passed << " Passed" << RESET << ", "
              << (g_db_failed > 0 ? RED : GREEN) << g_db_failed << " Failed" << RESET << "\n";
    std::cout << "========================================================" << RESET << "\n\n";

    return g_db_failed > 0 ? 1 : 0;
}

