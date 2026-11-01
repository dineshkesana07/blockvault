#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"
#include <sodium.h>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <atomic>
#include <cstring>
#include <algorithm>

namespace blockvault {

namespace {
    std::atomic<bool> g_sodium_initialized{false};
    std::mutex        g_init_mutex;
}

// =============================================================================
// 1. Core Lifecycle & Initialization
// =============================================================================

bool CryptoManager::initialize() {
    if (g_sodium_initialized.load(std::memory_order_acquire)) {
        return true;
    }

    std::lock_guard<std::mutex> lock(g_init_mutex);
    if (g_sodium_initialized.load(std::memory_order_relaxed)) {
        return true;
    }

    // sodium_init() returns 0 on initial success, 1 if already initialized, -1 on failure
    int res = sodium_init();
    if (res < 0) {
        throw CryptoException("Fatal: Failed to initialize libsodium cryptographic library");
    }

    g_sodium_initialized.store(true, std::memory_order_release);
    return true;
}

void CryptoManager::ensure_initialized() {
    if (!g_sodium_initialized.load(std::memory_order_acquire)) {
        initialize();
    }
}

// =============================================================================
// 2. Randomness & Nonce Generation (CSPRNG)
// =============================================================================

ByteVector CryptoManager::generateSalt(size_t length) {
    ensure_initialized();
    if (length == 0) {
        throw CryptoException("Salt length must be greater than zero");
    }
    ByteVector salt(length);
    randombytes_buf(salt.data(), salt.size());
    return salt;
}

ByteVector CryptoManager::generateNonce() {
    ensure_initialized();
    ByteVector nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    randombytes_buf(nonce.data(), nonce.size());
    return nonce;
}

ByteVector CryptoManager::generate_random_bytes(size_t length) {
    ensure_initialized();
    if (length == 0) return {};
    ByteVector bytes(length);
    randombytes_buf(bytes.data(), bytes.size());
    return bytes;
}

std::string CryptoManager::generate_random_hex(size_t byte_length) {
    ByteVector bytes = generate_random_bytes(byte_length);
    return to_hex(bytes);
}

std::string CryptoManager::generate_uuid_v4() {
    ensure_initialized();
    uint8_t bytes[16];
    randombytes_buf(bytes, sizeof(bytes));

    // RFC 4122 variant & version 4 bits
    bytes[6] = (bytes[6] & 0x0F) | 0x40; // Version 4
    bytes[8] = (bytes[8] & 0x3F) | 0x80; // Variant 1 (RFC 4122)

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            ss << '-';
        }
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
}

// =============================================================================
// 3. Argon2id Key Derivation (crypto_pwhash)
// =============================================================================

Argon2Config CryptoManager::get_argon2_config(Argon2Profile profile) {
    switch (profile) {
        case Argon2Profile::MODERATE:
            return {
                crypto_pwhash_OPSLIMIT_MODERATE,
                crypto_pwhash_MEMLIMIT_MODERATE
            };
        case Argon2Profile::SENSITIVE:
            return {
                crypto_pwhash_OPSLIMIT_SENSITIVE,
                crypto_pwhash_MEMLIMIT_SENSITIVE
            };
        case Argon2Profile::FAST_TEST:
            return {
                1ULL, // 1 iteration pass
                8ULL * 1024ULL * 1024ULL // 8 MB RAM
            };
        case Argon2Profile::INTERACTIVE:
        default:
            return {
                crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE
            };
    }
}

SecureBuffer CryptoManager::deriveKey(
    const SecureBuffer& password,
    const ByteVector& salt,
    size_t key_length,
    Argon2Profile profile) 
{
    Argon2Config cfg = get_argon2_config(profile);
    return deriveKey(password.to_string(), salt, key_length, cfg.opslimit, cfg.memlimit);
}

SecureBuffer CryptoManager::deriveKey(
    const std::string& password,
    const ByteVector& salt,
    size_t key_length,
    Argon2Profile profile) 
{
    Argon2Config cfg = get_argon2_config(profile);
    return deriveKey(password, salt, key_length, cfg.opslimit, cfg.memlimit);
}

SecureBuffer CryptoManager::deriveKey(
    const std::string& password,
    const ByteVector& salt,
    size_t key_length,
    unsigned long long opslimit,
    size_t memlimit) 
{
    ensure_initialized();

    if (salt.size() < crypto_pwhash_SALTBYTES) {
        throw CryptoException("Argon2id salt size is insufficient (minimum 16 bytes required)");
    }
    if (key_length == 0) {
        throw CryptoException("Derived key length must be greater than zero");
    }

    SecureBuffer derived_key(key_length);

    int res = crypto_pwhash(
        derived_key.data(),
        key_length,
        password.c_str(),
        password.length(),
        salt.data(),
        opslimit,
        memlimit,
        crypto_pwhash_ALG_ARGON2ID13
    );

    if (res != 0) {
        derived_key.zeroize();
        throw CryptoException("Argon2id key derivation failed (insufficient memory or invalid resource limits)");
    }

    return derived_key;
}

SecureBuffer CryptoManager::derive_key(
    const std::string& master_password,
    const ByteVector& salt,
    size_t key_length,
    uint32_t iterations) 
{
    // Fast path for test iterations if explicitly set low, otherwise interactive profile
    if (iterations > 0 && iterations <= 10000) {
        return deriveKey(master_password, salt, key_length, Argon2Profile::FAST_TEST);
    }
    return deriveKey(master_password, salt, key_length, Argon2Profile::INTERACTIVE);
}

// =============================================================================
// 4. Authenticated Encryption with Associated Data (XChaCha20-Poly1305)
// =============================================================================

EncryptedPayload CryptoManager::encrypt(
    const SecureBuffer& plaintext,
    const SecureBuffer& key,
    const std::string& associated_data) 
{
    ensure_initialized();

    if (key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
        throw CryptoException("Invalid encryption key length: expected 32 bytes (256-bit)");
    }

    ByteVector nonce = generateNonce();
    ByteVector ciphertext(plaintext.size());
    ByteVector tag(crypto_aead_xchacha20poly1305_ietf_ABYTES);

    unsigned long long actual_tag_len = 0;
    int res = crypto_aead_xchacha20poly1305_ietf_encrypt_detached(
        ciphertext.data(),
        tag.data(),
        &actual_tag_len,
        plaintext.data(),
        plaintext.size(),
        reinterpret_cast<const unsigned char*>(associated_data.data()),
        associated_data.size(),
        nullptr, // nsec is always NULL
        nonce.data(),
        key.data()
    );

    if (res != 0) {
        throw CryptoException("XChaCha20-Poly1305 detached encryption failed");
    }

    return EncryptedPayload{
        std::move(nonce),
        std::move(ciphertext),
        std::move(tag)
    };
}

EncryptedPayload CryptoManager::encrypt(
    const ByteVector& plaintext,
    const SecureBuffer& key,
    const ByteVector& nonce,
    const ByteVector& associated_data) 
{
    ensure_initialized();

    if (key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
        throw CryptoException("Invalid encryption key length: expected 32 bytes");
    }
    if (nonce.size() != crypto_aead_xchacha20poly1305_ietf_NPUBBYTES) {
        throw CryptoException("Invalid nonce length: expected 24 bytes for XChaCha20");
    }

    ByteVector ciphertext(plaintext.size());
    ByteVector tag(crypto_aead_xchacha20poly1305_ietf_ABYTES);

    unsigned long long actual_tag_len = 0;
    int res = crypto_aead_xchacha20poly1305_ietf_encrypt_detached(
        ciphertext.data(),
        tag.data(),
        &actual_tag_len,
        plaintext.data(),
        plaintext.size(),
        associated_data.data(),
        associated_data.size(),
        nullptr,
        nonce.data(),
        key.data()
    );

    if (res != 0) {
        throw CryptoException("XChaCha20-Poly1305 detached encryption failed");
    }

    return EncryptedPayload{
        nonce,
        std::move(ciphertext),
        std::move(tag)
    };
}

SecureBuffer CryptoManager::decrypt(
    const EncryptedPayload& payload,
    const SecureBuffer& key,
    const std::string& associated_data) 
{
    ByteVector ad_bytes(associated_data.begin(), associated_data.end());
    return decrypt(payload.ciphertext, payload.tag, payload.nonce, key, ad_bytes);
}

SecureBuffer CryptoManager::decrypt(
    const ByteVector& ciphertext,
    const ByteVector& tag,
    const ByteVector& nonce,
    const SecureBuffer& key,
    const ByteVector& associated_data) 
{
    ensure_initialized();

    if (key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
        throw CryptoException("Invalid decryption key length: expected 32 bytes");
    }
    if (nonce.size() != crypto_aead_xchacha20poly1305_ietf_NPUBBYTES) {
        throw CryptoException("Invalid nonce length: expected 24 bytes for XChaCha20");
    }
    if (tag.size() != crypto_aead_xchacha20poly1305_ietf_ABYTES) {
        throw CryptoException("Invalid Poly1305 authentication tag length: expected 16 bytes");
    }

    SecureBuffer plaintext(ciphertext.size());

    int res = crypto_aead_xchacha20poly1305_ietf_decrypt_detached(
        plaintext.data(),
        nullptr, // nsec is always NULL
        ciphertext.data(),
        ciphertext.size(),
        tag.data(),
        associated_data.data(),
        associated_data.size(),
        nonce.data(),
        key.data()
    );

    if (res != 0) {
        plaintext.zeroize();
        throw CryptoException("Ciphertext authentication failed: message corrupted, tampered with, or key invalid");
    }

    return plaintext;
}

EncryptedPayload CryptoManager::encrypt_string(
    const std::string& plaintext,
    const SecureBuffer& key,
    const std::string& associated_data) 
{
    SecureBuffer pbuf(plaintext);
    return encrypt(pbuf, key, associated_data);
}

std::string CryptoManager::decrypt_string(
    const EncryptedPayload& payload,
    const SecureBuffer& key,
    const std::string& associated_data) 
{
    SecureBuffer dec = decrypt(payload, key, associated_data);
    return dec.to_string();
}

ByteVector CryptoManager::encryptCombined(
    const SecureBuffer& plaintext,
    const SecureBuffer& key,
    const ByteVector& associated_data) 
{
    ensure_initialized();

    if (key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
        throw CryptoException("Invalid encryption key length");
    }

    ByteVector nonce = generateNonce();
    size_t combined_len = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES + 
                          plaintext.size() + 
                          crypto_aead_xchacha20poly1305_ietf_ABYTES;
    ByteVector out(combined_len);

    // Copy nonce to first 24 bytes
    std::memcpy(out.data(), nonce.data(), crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);

    unsigned long long actual_ct_len = 0;
    int res = crypto_aead_xchacha20poly1305_ietf_encrypt(
        out.data() + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES,
        &actual_ct_len,
        plaintext.data(),
        plaintext.size(),
        associated_data.data(),
        associated_data.size(),
        nullptr,
        nonce.data(),
        key.data()
    );

    if (res != 0) {
        throw CryptoException("XChaCha20-Poly1305 combined encryption failed");
    }

    return out;
}

SecureBuffer CryptoManager::decryptCombined(
    const ByteVector& combined_payload,
    const SecureBuffer& key,
    const ByteVector& associated_data) 
{
    ensure_initialized();

    constexpr size_t min_len = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES + 
                               crypto_aead_xchacha20poly1305_ietf_ABYTES;
    if (combined_payload.size() < min_len) {
        throw CryptoException("Combined payload too short to contain valid nonce and MAC tag");
    }
    if (key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
        throw CryptoException("Invalid decryption key length");
    }

    const uint8_t* nonce_ptr = combined_payload.data();
    const uint8_t* ct_ptr = combined_payload.data() + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
    size_t ct_len = combined_payload.size() - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

    size_t plaintext_len = ct_len - crypto_aead_xchacha20poly1305_ietf_ABYTES;
    SecureBuffer plaintext(plaintext_len);

    unsigned long long actual_pt_len = 0;
    int res = crypto_aead_xchacha20poly1305_ietf_decrypt(
        plaintext.data(),
        &actual_pt_len,
        nullptr,
        ct_ptr,
        ct_len,
        associated_data.data(),
        associated_data.size(),
        nonce_ptr,
        key.data()
    );

    if (res != 0) {
        plaintext.zeroize();
        throw CryptoException("Combined ciphertext authentication failed: data corrupted or key invalid");
    }

    return plaintext;
}

// =============================================================================
// 5. Memory Security & Zeroization
// =============================================================================

void CryptoManager::clearSensitiveMemory(void* ptr, size_t length) noexcept {
    if (ptr && length > 0) {
        sodium_memzero(ptr, length);
    }
}

void CryptoManager::clearSensitiveMemory(std::string& str) noexcept {
    if (!str.empty()) {
        sodium_memzero(str.data(), str.size());
        str.clear();
    }
}

void CryptoManager::clearSensitiveMemory(ByteVector& vec) noexcept {
    if (!vec.empty()) {
        sodium_memzero(vec.data(), vec.size());
        vec.clear();
    }
}

void CryptoManager::clearSensitiveMemory(SecureBuffer& buf) noexcept {
    buf.zeroize();
    buf.clear();
}

// =============================================================================
// 6. Cryptographic Hash & Verification Helpers
// =============================================================================

ByteVector CryptoManager::sha256_bytes(const ByteVector& input) {
    ensure_initialized();
    ByteVector digest(crypto_hash_sha256_BYTES);
    crypto_hash_sha256(digest.data(), input.data(), input.size());
    return digest;
}

std::string CryptoManager::sha256_hex(const ByteVector& input) {
    ByteVector digest = sha256_bytes(input);
    return to_hex(digest);
}

std::string CryptoManager::sha256_hex(const std::string& input) {
    ensure_initialized();
    ByteVector digest(crypto_hash_sha256_BYTES);
    crypto_hash_sha256(
        digest.data(),
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size()
    );
    return to_hex(digest);
}

bool CryptoManager::constant_time_equals(const std::string& a, const std::string& b) noexcept {
    if (a.size() != b.size()) return false;
    return constant_time_equals(
        reinterpret_cast<const uint8_t*>(a.data()),
        reinterpret_cast<const uint8_t*>(b.data()),
        a.size()
    );
}

bool CryptoManager::constant_time_equals(const ByteVector& a, const ByteVector& b) noexcept {
    if (a.size() != b.size()) return false;
    return constant_time_equals(a.data(), b.data(), a.size());
}

bool CryptoManager::constant_time_equals(const uint8_t* a, const uint8_t* b, size_t len) noexcept {
    if (len == 0) return true;
    if (!a || !b) return false;
    return sodium_memcmp(a, b, len) == 0;
}

// =============================================================================
// 7. Hex Encoding Helpers
// =============================================================================

std::string CryptoManager::to_hex(const ByteVector& bytes) {
    return to_hex(bytes.data(), bytes.size());
}

std::string CryptoManager::to_hex(const uint8_t* data, size_t length) {
    if (!data || length == 0) return "";
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

ByteVector CryptoManager::from_hex(const std::string& hex) {
    if (hex.length() % 2 != 0) {
        throw CryptoException("Invalid hex string length for decoding (must be even)");
    }
    ByteVector bytes;
    bytes.reserve(hex.length() / 2);
    for (size_t i = 0; i < hex.length(); i += 2) {
        char high = hex[i];
        char low  = hex[i + 1];

        auto hex_val = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };

        int h = hex_val(high);
        int l = hex_val(low);
        if (h == -1 || l == -1) {
            throw CryptoException("Invalid non-hexadecimal character encountered during decoding");
        }
        bytes.push_back(static_cast<uint8_t>((h << 4) | l));
    }
    return bytes;
}

} // namespace blockvault
