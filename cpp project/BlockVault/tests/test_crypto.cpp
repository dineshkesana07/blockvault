#include "crypto/CryptoManager.hpp"
#include "crypto/SecureBuffer.hpp"
#include "common/Exceptions.hpp"
#include <sodium.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include <string>
#include <set>

using namespace blockvault;

// Simple ANSI color helpers
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define RUN_TEST(test_func) \
    do { \
        std::cout << "[CRYPTO TEST] " << #test_func << " ... "; \
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

// =============================================================================
// Test 1: Libsodium Initialization and Health Check
// =============================================================================
void test_initialization() {
    bool ok = CryptoManager::initialize();
    assert(ok == true);
    // Idempotent secondary call
    bool ok2 = CryptoManager::initialize();
    assert(ok2 == true);
    assert(sodium_version_string() != nullptr);
}

// =============================================================================
// Test 2: CSPRNG Randomness and Entropy Generation
// =============================================================================
void test_csprng_and_entropy() {
    // 1. Salt Generation
    ByteVector salt1 = CryptoManager::generateSalt(16);
    ByteVector salt2 = CryptoManager::generateSalt(16);
    assert(salt1.size() == 16);
    assert(salt2.size() == 16);
    assert(!CryptoManager::constant_time_equals(salt1, salt2));

    // 2. Nonce Generation (24 bytes for XChaCha20)
    ByteVector nonce1 = CryptoManager::generateNonce();
    ByteVector nonce2 = CryptoManager::generateNonce();
    assert(nonce1.size() == 24);
    assert(nonce2.size() == 24);
    assert(!CryptoManager::constant_time_equals(nonce1, nonce2));

    // 3. Uniqueness over 100 nonces (no collisions)
    std::set<std::string> seen_nonces;
    for (int i = 0; i < 100; ++i) {
        std::string hex_n = CryptoManager::to_hex(CryptoManager::generateNonce());
        assert(seen_nonces.find(hex_n) == seen_nonces.end());
        seen_nonces.insert(hex_n);
    }

    // 4. UUID v4 Format Check
    std::string uuid = CryptoManager::generate_uuid_v4();
    assert(uuid.length() == 36);
    assert(uuid[8] == '-' && uuid[13] == '-' && uuid[18] == '-' && uuid[23] == '-');
    assert(uuid[14] == '4'); // Version 4
    char variant = uuid[19]; // Variant 1 (8, 9, a, b)
    assert(variant == '8' || variant == '9' || variant == 'a' || variant == 'b');
}

// =============================================================================
// Test 3: Argon2id Key Derivation (crypto_pwhash)
// =============================================================================
void test_argon2id_derivation() {
    std::string password = "CorrectHorseBatteryStaple#2026!";
    ByteVector salt = CryptoManager::generateSalt(16);

    // 1. Deterministic derivation test: same password + salt produces identical key
    SecureBuffer key1 = CryptoManager::deriveKey(password, salt, 32, Argon2Profile::FAST_TEST);
    SecureBuffer key2 = CryptoManager::deriveKey(password, salt, 32, Argon2Profile::FAST_TEST);
    assert(key1.size() == 32);
    assert(key2.size() == 32);
    assert(CryptoManager::constant_time_equals(key1.to_vector(), key2.to_vector()));

    // 2. Avalanche effect: 1-character difference in password produces completely different key
    std::string alt_password = "CorrectHorseBatteryStaple#2026?";
    SecureBuffer key_alt = CryptoManager::deriveKey(alt_password, salt, 32, Argon2Profile::FAST_TEST);
    assert(!CryptoManager::constant_time_equals(key1.to_vector(), key_alt.to_vector()));

    // 3. Salt distinction: Same password with different salt produces completely different key
    ByteVector salt_diff = CryptoManager::generateSalt(16);
    SecureBuffer key_salt_diff = CryptoManager::deriveKey(password, salt_diff, 32, Argon2Profile::FAST_TEST);
    assert(!CryptoManager::constant_time_equals(key1.to_vector(), key_salt_diff.to_vector()));

    // 4. Error handling: salt too small throws CryptoException
    ByteVector tiny_salt = { 0x01, 0x02, 0x03 };
    bool threw = false;
    try {
        (void)CryptoManager::deriveKey(password, tiny_salt, 32, Argon2Profile::FAST_TEST);
    } catch (const CryptoException&) {
        threw = true;
    }
    assert(threw == true);
}

// =============================================================================
// Test 4: XChaCha20-Poly1305 Authenticated Encryption Roundtrips
// =============================================================================
void test_aead_positive_roundtrips() {
    SecureBuffer key(CryptoManager::generate_random_bytes(32));

    // 1. Standard credential password payload
    std::string secret = "P@ssw0rd!Secure#2026Enterprise";
    EncryptedPayload enc = CryptoManager::encrypt_string(secret, key, "METADATA_URL=https://bank.com");
    assert(enc.nonce.size() == 24);
    assert(enc.tag.size() == 16);
    assert(enc.ciphertext.size() == secret.length());

    std::string dec = CryptoManager::decrypt_string(enc, key, "METADATA_URL=https://bank.com");
    assert(dec == secret);

    // 2. Empty plaintext roundtrip
    EncryptedPayload empty_enc = CryptoManager::encrypt_string("", key, "");
    assert(empty_enc.ciphertext.empty());
    assert(empty_enc.tag.size() == 16);
    std::string empty_dec = CryptoManager::decrypt_string(empty_enc, key, "");
    assert(empty_dec.empty());

    // 3. Large payload (64 KB secure file / note)
    std::string large_doc(65536, 'X');
    for (size_t i = 0; i < large_doc.size(); ++i) {
        large_doc[i] = static_cast<char>('A' + (i % 26));
    }
    EncryptedPayload large_enc = CryptoManager::encrypt_string(large_doc, key, "LARGE_DOC");
    std::string large_dec = CryptoManager::decrypt_string(large_enc, key, "LARGE_DOC");
    assert(large_dec == large_doc);

    // 4. Unicode & Multi-Byte UTF-8 characters
    std::string utf8_text = "Master Password: 🔐 パスワード 密码 пароль كلمة المرور 🛡️";
    EncryptedPayload utf8_enc = CryptoManager::encrypt_string(utf8_text, key, "UNICODE_TEST");
    std::string utf8_dec = CryptoManager::decrypt_string(utf8_enc, key, "UNICODE_TEST");
    assert(utf8_dec == utf8_text);

    // 5. Combined Ciphertext API ([nonce | ciphertext | tag])
    SecureBuffer sec_plaintext(utf8_text);
    ByteVector combined = CryptoManager::encryptCombined(sec_plaintext, key, {0x42, 0x43});
    assert(combined.size() == 24 + utf8_text.size() + 16);
    SecureBuffer dec_combined = CryptoManager::decryptCombined(combined, key, {0x42, 0x43});
    assert(dec_combined.to_string() == utf8_text);
}

// =============================================================================
// Test 5: AEAD Negative Tests (Wrong Key, Wrong Nonce, Bad AAD)
// =============================================================================
void test_aead_negative_cases() {
    SecureBuffer key_correct(CryptoManager::generate_random_bytes(32));
    SecureBuffer key_wrong(CryptoManager::generate_random_bytes(32));

    std::string plaintext = "Confidential Banking Token #999888";
    EncryptedPayload enc = CryptoManager::encrypt_string(plaintext, key_correct, "USER_SESSION=42");

    // Case 1: Decryption with wrong key must fail and throw CryptoException
    bool wrong_key_threw = false;
    try {
        (void)CryptoManager::decrypt_string(enc, key_wrong, "USER_SESSION=42");
    } catch (const CryptoException&) {
        wrong_key_threw = true;
    }
    assert(wrong_key_threw == true);

    // Case 2: Decryption with wrong nonce must fail
    EncryptedPayload bad_nonce_enc = enc;
    bad_nonce_enc.nonce = CryptoManager::generateNonce();
    bool wrong_nonce_threw = false;
    try {
        (void)CryptoManager::decrypt_string(bad_nonce_enc, key_correct, "USER_SESSION=42");
    } catch (const CryptoException&) {
        wrong_nonce_threw = true;
    }
    assert(wrong_nonce_threw == true);

    // Case 3: Decryption with mismatched Associated Data (AAD) must fail
    bool wrong_aad_threw = false;
    try {
        // Encrypted with USER_SESSION=42, attempted with USER_SESSION=99
        (void)CryptoManager::decrypt_string(enc, key_correct, "USER_SESSION=99");
    } catch (const CryptoException&) {
        wrong_aad_threw = true;
    }
    assert(wrong_aad_threw == true);

    // Case 4: Decryption with omitted AAD when one was provided at encryption
    bool omitted_aad_threw = false;
    try {
        (void)CryptoManager::decrypt_string(enc, key_correct, "");
    } catch (const CryptoException&) {
        omitted_aad_threw = true;
    }
    assert(omitted_aad_threw == true);
}

// =============================================================================
// Test 6: Tampering & Bit-Flip Attacks
// =============================================================================
void test_tampering_attacks() {
    SecureBuffer key(CryptoManager::generate_random_bytes(32));
    std::string sensitive_data = "TRANSFER: $500,000 -> ACCOUNT_B";
    EncryptedPayload enc = CryptoManager::encrypt_string(sensitive_data, key, "TRANSACTION_ID=1001");

    // 1. Bit-flip in ciphertext payload (modify 'B' to 'A')
    EncryptedPayload tampered_ct = enc;
    tampered_ct.ciphertext[tampered_ct.ciphertext.size() - 1] ^= 0x01; // flip lowest bit
    bool ct_tamper_caught = false;
    try {
        (void)CryptoManager::decrypt_string(tampered_ct, key, "TRANSACTION_ID=1001");
    } catch (const CryptoException&) {
        ct_tamper_caught = true;
    }
    assert(ct_tamper_caught == true);

    // 2. Bit-flip in Poly1305 authentication tag
    EncryptedPayload tampered_tag = enc;
    tampered_tag.tag[0] ^= 0x80; // flip highest bit of first tag byte
    bool tag_tamper_caught = false;
    try {
        (void)CryptoManager::decrypt_string(tampered_tag, key, "TRANSACTION_ID=1001");
    } catch (const CryptoException&) {
        tag_tamper_caught = true;
    }
    assert(tag_tamper_caught == true);

    // 3. Truncated ciphertext payload
    EncryptedPayload truncated_payload = enc;
    truncated_payload.ciphertext.pop_back();
    bool trunc_caught = false;
    try {
        (void)CryptoManager::decrypt_string(truncated_payload, key, "TRANSACTION_ID=1001");
    } catch (const CryptoException&) {
        trunc_caught = true;
    }
    assert(trunc_caught == true);
}

// =============================================================================
// Test 7: SecureBuffer Memory Lifecycle & Protection
// =============================================================================
void test_secure_buffer_lifecycle() {
    // 1. Allocation & Zeroization
    SecureBuffer buf(64);
    assert(buf.size() == 64);
    assert(buf.data() != nullptr);

    // Populate with test pattern
    for (size_t i = 0; i < buf.size(); ++i) {
        buf[i] = static_cast<uint8_t>(0xAA);
    }
    assert(buf[0] == 0xAA);

    // Zeroize buffer
    buf.zeroize();
    for (size_t i = 0; i < buf.size(); ++i) {
        assert(buf[i] == 0);
    }

    // 2. Move Semantics
    SecureBuffer source(32);
    source[0] = 0x77;
    uint8_t* source_ptr = source.data();

    SecureBuffer dest = std::move(source);
    assert(source.empty());
    assert(source.data() == nullptr);
    assert(source.size() == 0);
    assert(dest.size() == 32);
    assert(dest.data() == source_ptr);
    assert(dest[0] == 0x77);

    // 3. Constant-Time Memory Comparison
    ByteVector b1 = { 0x01, 0x02, 0x03, 0x04 };
    ByteVector b2 = { 0x01, 0x02, 0x03, 0x04 };
    ByteVector b3 = { 0x01, 0x02, 0x03, 0x05 };
    assert(CryptoManager::constant_time_equals(b1, b2) == true);
    assert(CryptoManager::constant_time_equals(b1, b3) == false);

    // 4. Memory Protection (make_readonly / make_readwrite)
    SecureBuffer prot(32);
    prot[0] = 0x42;
    prot.make_readonly();
    assert(prot[0] == 0x42);
    prot.make_readwrite();
    prot[0] = 0x43;
    assert(prot[0] == 0x43);
}

// =============================================================================
// Test 8: Key Rotation & Envelope Re-encryption Workflow
// =============================================================================
void test_key_rotation_envelope() {
    // Scenario: User changes master password from OldPass to NewPass.
    // The vault DEK (Data Encryption Key) must be safely re-encrypted under the new KEK
    // without re-encrypting the underlying vault credentials.

    std::string old_master_pass = "OldMasterPassword#2025!";
    std::string new_master_pass = "NewStrongerPassword#2026!";

    // Generate initial registration material
    ByteVector old_salt = CryptoManager::generateSalt(16);
    SecureBuffer old_kek = CryptoManager::deriveKey(old_master_pass, old_salt, 32, Argon2Profile::FAST_TEST);

    // Generate random 256-bit vault DEK
    SecureBuffer vault_dek(CryptoManager::generate_random_bytes(32));

    // Wrap DEK with old KEK
    EncryptedPayload old_dek_envelope = CryptoManager::encrypt(vault_dek, old_kek, "USER_DEK_ENVELOPE");

    // Encrypt a credential using the DEK
    std::string bank_secret = "ChaseBankPIN#7788";
    EncryptedPayload credential_ct = CryptoManager::encrypt_string(bank_secret, vault_dek, "CREDENTIAL_ID=1");

    // --- EXECUTE KEY ROTATION ---
    // 1. Unlock / unwrap DEK using old credentials
    SecureBuffer unwrapped_dek = CryptoManager::decrypt(old_dek_envelope, old_kek, "USER_DEK_ENVELOPE");
    assert(CryptoManager::constant_time_equals(unwrapped_dek.to_vector(), vault_dek.to_vector()));

    // 2. Derive new KEK with new salt from new password
    ByteVector new_salt = CryptoManager::generateSalt(16);
    SecureBuffer new_kek = CryptoManager::deriveKey(new_master_pass, new_salt, 32, Argon2Profile::FAST_TEST);

    // 3. Re-wrap unwrapped DEK under new KEK
    EncryptedPayload new_dek_envelope = CryptoManager::encrypt(unwrapped_dek, new_kek, "USER_DEK_ENVELOPE");

    // 4. Securely clear old keys from memory
    CryptoManager::clearSensitiveMemory(old_kek);
    CryptoManager::clearSensitiveMemory(unwrapped_dek);

    // --- VERIFY RECOVERY AFTER ROTATION ---
    // Decrypt DEK using new master password
    SecureBuffer recovered_dek = CryptoManager::decrypt(new_dek_envelope, new_kek, "USER_DEK_ENVELOPE");
    
    // Decrypt the existing credential using recovered DEK (credential was NOT re-encrypted!)
    std::string recovered_secret = CryptoManager::decrypt_string(credential_ct, recovered_dek, "CREDENTIAL_ID=1");
    assert(recovered_secret == bank_secret);
}

// =============================================================================
// Main Test Runner Entrypoint
// =============================================================================
int main() {
    std::cout << "\n========================================================\n";
    std::cout << " BLOCKVAULT C++20 LIBSODIUM CRYPTOGRAPHY TEST SUITE\n";
    std::cout << "========================================================\n\n";

    RUN_TEST(test_initialization);
    RUN_TEST(test_csprng_and_entropy);
    RUN_TEST(test_argon2id_derivation);
    RUN_TEST(test_aead_positive_roundtrips);
    RUN_TEST(test_aead_negative_cases);
    RUN_TEST(test_tampering_attacks);
    RUN_TEST(test_secure_buffer_lifecycle);
    RUN_TEST(test_key_rotation_envelope);

    std::cout << "\n--------------------------------------------------------\n";
    std::cout << " CRYPTOGRAPHY RESULTS: " 
              << g_tests_passed << " Passed, " 
              << g_tests_failed << " Failed\n";
    std::cout << "========================================================\n\n";

    return (g_tests_failed == 0) ? 0 : 1;
}

