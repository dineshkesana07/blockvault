#include "crypto/SecureBuffer.hpp"
#include "crypto/CryptoManager.hpp"
#include "crypto/PasswordGenerator.hpp"
#include "models/User.hpp"
#include "models/Credential.hpp"
#include "models/Block.hpp"
#include "models/Vault.hpp"
#include "db/DatabaseManager.hpp"
#include "ledger/AuditLedger.hpp"
#include "auth/SessionManager.hpp"
#include "auth/AuthManager.hpp"
#include "vault/CredentialManager.hpp"
#include "common/Exceptions.hpp"

#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <string>

using namespace blockvault;

// ANSI color codes for terminal output
#define GREEN "\033[32m"
#define RED   "\033[31m"
#define CYAN  "\033[36m"
#define BOLD  "\033[1m"
#define RESET "\033[0m"

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define BV_TEST(name) \
    std::cout << CYAN << "[TEST] " << RESET << name << " ... "; \
    try {

#define BV_PASS() \
        std::cout << GREEN << BOLD << "PASSED" << RESET << "\n"; \
        g_tests_passed++; \
    } catch (const std::exception& e) { \
        std::cout << RED << BOLD << "FAILED (" << e.what() << ")" << RESET << "\n"; \
        g_tests_failed++; \
    }

// -----------------------------------------------------------------------------
// Test 1: SecureBuffer Memory Lifecycle & RAII
// -----------------------------------------------------------------------------
void test_secure_buffer() {
    BV_TEST("SecureBuffer Allocation, Move & Zeroization")
        SecureBuffer buf(32);
        assert(buf.size() == 32);
        assert(!buf.empty());

        // Fill with test pattern
        for (size_t i = 0; i < 32; ++i) {
            buf[i] = static_cast<uint8_t>(i + 1);
        }

        // Test move constructor
        SecureBuffer moved_buf(std::move(buf));
        assert(buf.empty());
        assert(moved_buf.size() == 32);
        assert(moved_buf[0] == 1);
        assert(moved_buf[31] == 32);

        // Test zeroization
        moved_buf.zeroize();
        for (size_t i = 0; i < 32; ++i) {
            assert(moved_buf[i] == 0);
        }
    BV_PASS()
}

// -----------------------------------------------------------------------------
// Test 2: CryptoManager CSPRNG, SHA-256 & AEAD Encryption
// -----------------------------------------------------------------------------
void test_crypto_manager() {
    BV_TEST("CryptoManager SHA-256, CSPRNG & AEAD Roundtrip")
        // CSPRNG
        ByteVector rand_bytes = CryptoManager::generate_random_bytes(32);
        assert(rand_bytes.size() == 32);

        std::string uuid = CryptoManager::generate_uuid_v4();
        assert(uuid.length() == 36);
        assert(uuid[14] == '4'); // Version 4

        // SHA-256 standard test vector: SHA-256("hello")
        std::string hash = CryptoManager::sha256_hex("hello");
        assert(hash == "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");

        // Key Derivation
        ByteVector salt = CryptoManager::generate_random_bytes(16);
        SecureBuffer key = CryptoManager::derive_key("SuperSecretMasterPass123!", salt, 32, 1000);
        assert(key.size() == 32);

        // AEAD Encryption / Decryption Roundtrip
        std::string secret_message = "Chase Bank Master Password: P@ssw0rd987#X!";
        EncryptedPayload payload = CryptoManager::encrypt_string(secret_message, key, "AAD_METADATA");
        assert(payload.nonce.size() == NONCE_BYTES);
        assert(payload.tag.size() == MAC_BYTES);
        assert(!payload.ciphertext.empty());

        std::string decrypted = CryptoManager::decrypt_string(payload, key, "AAD_METADATA");
        assert(decrypted == secret_message);

        // Bit-Flipping Tampering Detection
        EncryptedPayload tampered = payload;
        tampered.ciphertext[0] ^= 0xFF; // Corrupt single bit
        bool caught = false;
        try {
            (void)CryptoManager::decrypt_string(tampered, key, "AAD_METADATA");
        } catch (const CryptoException&) {
            caught = true;
        }
        assert(caught);
    BV_PASS()
}

// -----------------------------------------------------------------------------
// Test 3: PasswordGenerator Entropy & Mnemonic Recovery Phrase
// -----------------------------------------------------------------------------
void test_password_generator() {
    BV_TEST("PasswordGenerator Policy, Shannon Entropy & 24-Word Recovery")
        PasswordOptions opts;
        opts.length = 20;
        opts.include_uppercase = true;
        opts.include_lowercase = true;
        opts.include_numbers = true;
        opts.include_symbols = true;
        opts.avoid_ambiguous = true;

        std::string pwd = PasswordGenerator::generate(opts);
        assert(pwd.length() == 20);

        PasswordAssessment assessment = PasswordGenerator::assess(pwd);
        assert(assessment.entropy_bits > 80.0);
        assert(assessment.strength >= PasswordStrengthLevel::Strong);

        // 24-word recovery phrase
        std::vector<std::string> phrase = PasswordGenerator::generate_recovery_phrase();
        assert(phrase.size() == 24);
        for (const auto& word : phrase) {
            assert(!word.empty());
        }
    BV_PASS()
}

// -----------------------------------------------------------------------------
// Test 4: DatabaseManager In-Memory SQLite CRUD
// -----------------------------------------------------------------------------
void test_database_manager() {
    BV_TEST("DatabaseManager SQLite Schema & Entity Persistence")
        DatabaseManager db(":memory:");
        assert(db.is_open());

        // Insert and read User
        ByteVector salt = CryptoManager::generate_random_bytes(16);
        EncryptedPayload dummy_dek;
        dummy_dek.nonce = CryptoManager::generate_random_bytes(24);
        dummy_dek.tag = CryptoManager::generate_random_bytes(16);
        dummy_dek.ciphertext = CryptoManager::generate_random_bytes(32);

        User user("u-1234", "alice@blockvault.sec", "hash_abc", salt, dummy_dek);
        db.insert_user(user);

        std::optional<User> fetched = db.get_user_by_email("alice@blockvault.sec");
        assert(fetched.has_value());
        assert(fetched->id() == "u-1234");
        assert(fetched->email() == "alice@blockvault.sec");

        // Insert and read Credential
        Credential cred(
            "c-5678", "u-1234", "AWS Console", "https://aws.amazon.com", "root",
            dummy_dek, dummy_dek, CredentialCategory::Server, true
        );
        db.insert_credential(cred);

        std::optional<Credential> fetched_cred = db.get_credential_by_id("c-5678", "u-1234");
        assert(fetched_cred.has_value());
        assert(fetched_cred->title() == "AWS Console");
        assert(fetched_cred->category() == CredentialCategory::Server);
        assert(fetched_cred->is_favorite() == true);
    BV_PASS()
}

// -----------------------------------------------------------------------------
// Test 5: AuditLedger Hash Chain & Tamper Evidence
// -----------------------------------------------------------------------------
void test_audit_ledger() {
    BV_TEST("AuditLedger Hash Chaining & Cryptographic Tamper Detection")
        auto db = std::make_shared<DatabaseManager>(":memory:");
        AuditLedger ledger(db);

        // Genesis block auto-created
        assert(ledger.get_total_records() == 1);
        IntegrityVerificationResult res1 = ledger.verify_integrity();
        assert(res1.is_valid);
        assert(res1.total_blocks == 1);

        // Add 5 security transactions
        ledger.record_event("u-01", AuditEventType::UserRegistered, "u-01", "User registration");
        ledger.record_event("u-01", AuditEventType::UserLoginSuccess, "u-01", "Login from 127.0.0.1");
        ledger.record_event("u-01", AuditEventType::CredentialCreated, "c-101", "Created GitHub");
        ledger.record_event("u-01", AuditEventType::CredentialRead, "c-101", "Read GitHub password");
        ledger.record_event("u-01", AuditEventType::VaultLocked, "u-01", "Vault locked");

        assert(ledger.get_total_records() == 6);
        IntegrityVerificationResult res2 = ledger.verify_integrity();
        assert(res2.is_valid);
        assert(res2.total_blocks == 6);

        // Malicious Tampering: corrupt an intermediate block in SQLite directly
        db->execute_raw("UPDATE audit_records SET event_metadata = 'HACKED EVENT' WHERE record_id = 4;");

        // Verification must detect the tampering and fail!
        IntegrityVerificationResult res_tampered = ledger.verify_integrity();
        assert(!res_tampered.is_valid);
        assert(res_tampered.tampered_index == 3);
    BV_PASS()
}

// -----------------------------------------------------------------------------
// Test 6: End-to-End Zero-Knowledge Auth & Credential Management
// -----------------------------------------------------------------------------
void test_end_to_end_vault() {
    BV_TEST("End-to-End Zero-Knowledge Auth & Credential CRUD")
        auto db = std::make_shared<DatabaseManager>(":memory:");
        auto session_mgr = std::make_shared<SessionManager>(db, 900);
        auto ledger = std::make_shared<AuditLedger>(db);

        AuthManager auth(db, session_mgr, ledger);
        CredentialManager cred_mgr(db, session_mgr, ledger);

        // 1. User Registration
        std::string master_pass = "Correct-Horse-Battery-Staple-2026!";
        User user = auth.register_user("dinesh@cyber.vault", master_pass);
        assert(user.email() == "dinesh@cyber.vault");

        // 2. Login
        std::string token = auth.login("dinesh@cyber.vault", master_pass, "192.168.1.50");
        assert(!token.empty());
        assert(session_mgr->is_vault_unlocked(token));

        // 3. Create Credentials
        Credential c1 = cred_mgr.create_credential(
            token, "GitHub", "https://github.com", "dineshbabu",
            "GhP_Secr3tToken!", "Personal access token with repo scope",
            CredentialCategory::Login, true
        );
        assert(!c1.id().empty());

        Credential c2 = cred_mgr.create_credential(
            token, "ProtonMail", "https://mail.proton.me", "dinesh@pm.me",
            "Pr0t0nM@il#99!", "Encrypted mail",
            CredentialCategory::Login, false
        );

        // 4. Verify Database Contains Only Ciphertext, Zero Plaintext
        std::optional<Credential> raw_c1 = db->get_credential_by_id(c1.id(), user.id());
        assert(raw_c1.has_value());
        // Plaintext must NOT appear in raw ciphertext blob
        std::string raw_pass_blob(raw_c1->encrypted_password().ciphertext.begin(), raw_c1->encrypted_password().ciphertext.end());
        assert(raw_pass_blob.find("GhP_Secr3tToken!") == std::string::npos);

        // 5. Decrypt Credential via Session
        DecryptedCredential decrypted = cred_mgr.get_decrypted_credential(token, c1.id());
        assert(decrypted.title == "GitHub");
        assert(decrypted.username == "dineshbabu");
        assert(decrypted.password == "GhP_Secr3tToken!");
        assert(decrypted.notes == "Personal access token with repo scope");

        // 6. Test Vault Domain Container
        Vault user_vault = cred_mgr.get_vault(token);
        assert(user_vault.size() == 2);
        assert(user_vault.favorites().size() == 1);
        assert(user_vault.search("proton").size() == 1);

        // 7. Lock Vault -> Decrypt attempts must fail
        auth.lock_vault(token);
        assert(!session_mgr->is_vault_unlocked(token));
        bool decrypt_blocked = false;
        try {
            (void)cred_mgr.get_decrypted_credential(token, c1.id());
        } catch (const AuthException&) {
            decrypt_blocked = true;
        }
        assert(decrypt_blocked);

        // 8. Re-Unlock with Master Password -> Decrypt works again
        auth.unlock_vault(token, master_pass);
        assert(session_mgr->is_vault_unlocked(token));
        DecryptedCredential decrypted_again = cred_mgr.get_decrypted_credential(token, c1.id());
        assert(decrypted_again.password == "GhP_Secr3tToken!");

        // 9. Verify Final Audit Ledger
        IntegrityVerificationResult audit_res = ledger->verify_integrity();
        assert(audit_res.is_valid);
        assert(ledger->get_total_records() >= 8);
    BV_PASS()
}

// -----------------------------------------------------------------------------
// Main Test Runner
// -----------------------------------------------------------------------------
int main() {
    std::cout << "\n" << BOLD << "========================================================\n";
    std::cout << " BLOCKVAULT C++20 CORE ARCHITECTURE TEST SUITE\n";
    std::cout << "========================================================" << RESET << "\n\n";

    test_secure_buffer();
    test_crypto_manager();
    test_password_generator();
    test_database_manager();
    test_audit_ledger();
    test_end_to_end_vault();

    std::cout << "\n" << BOLD << "--------------------------------------------------------\n";
    std::cout << " RESULTS: "
              << GREEN << g_tests_passed << " Passed" << RESET << ", "
              << (g_tests_failed > 0 ? RED : GREEN) << g_tests_failed << " Failed" << RESET << "\n";
    std::cout << "========================================================" << RESET << "\n\n";

    return g_tests_failed > 0 ? 1 : 0;
}
