#pragma once

#include "common/Types.hpp"
#include "crypto/SecureBuffer.hpp"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace blockvault {

/**
 * @enum Argon2Profile
 * @brief Resource profiles for Argon2id password-based key derivation.
 */
enum class Argon2Profile {
    INTERACTIVE, ///< Fast enough for interactive logins (~64 MB RAM, 2 passes)
    MODERATE,    ///< Moderate protection (~256 MB RAM, 3 passes)
    SENSITIVE,   ///< Maximum protection (~1024 MB RAM, 4 passes)
    FAST_TEST    ///< Fast test profile for CI/automated testing (8 MB RAM, 1 pass)
};

/**
 * @struct Argon2Config
 * @brief Explicit computational limits for Argon2id key derivation.
 */
struct Argon2Config {
    unsigned long long opslimit;
    size_t memlimit;
};

/**
 * @class CryptoManager
 * @brief Enterprise cryptographic services manager powered by libsodium.
 * 
 * Provides:
 * - Thread-safe libsodium initialization.
 * - Cryptographically secure pseudo-random number generation (CSPRNG).
 * - Memory-hard Argon2id key derivation via crypto_pwhash.
 * - Authenticated symmetric encryption (AEAD) via XChaCha20-Poly1305.
 * - Constant-time comparisons and secure memory clearing.
 * - Key lifecycle management and key rotation support.
 */
class CryptoManager {
public:
    CryptoManager() = delete; // Pure static utility class
    ~CryptoManager() = delete;

    // Prohibit copying and moving
    CryptoManager(const CryptoManager&) = delete;
    CryptoManager& operator=(const CryptoManager&) = delete;

    // =========================================================================
    // 1. Core Lifecycle & Initialization
    // =========================================================================

    /**
     * @brief Safely initializes the libsodium library in a thread-safe manner.
     * @return true if initialized successfully or already initialized.
     * @throws CryptoException if initialization fails.
     */
    static bool initialize();

    /**
     * @brief Ensures libsodium is initialized, throwing on failure.
     */
    static void ensure_initialized();

    // =========================================================================
    // 2. Randomness & Nonce Generation (CSPRNG)
    // =========================================================================

    /**
     * @brief Generates a cryptographically secure random salt for Argon2id.
     * @param length Number of bytes to generate (defaults to 16 bytes).
     * @return ByteVector containing random salt bytes.
     */
    [[nodiscard]] static ByteVector generateSalt(size_t length = ARGON2_SALT_BYTES);
    [[nodiscard]] static ByteVector generate_salt(size_t length = ARGON2_SALT_BYTES) {
        return generateSalt(length);
    }

    /**
     * @brief Generates a secure random 24-byte (192-bit) nonce for XChaCha20-Poly1305.
     * @return ByteVector containing 24 random nonce bytes.
     */
    [[nodiscard]] static ByteVector generateNonce();
    [[nodiscard]] static ByteVector generate_nonce() {
        return generateNonce();
    }

    /**
     * @brief Generates cryptographically secure random bytes of specified length.
     */
    [[nodiscard]] static ByteVector generate_random_bytes(size_t length);

    /**
     * @brief Generates random hex-encoded string of specified byte length.
     */
    [[nodiscard]] static std::string generate_random_hex(size_t byte_length);

    /**
     * @brief Generates a random RFC 4122 Version 4 UUID string.
     */
    [[nodiscard]] static std::string generate_uuid_v4();

    // =========================================================================
    // 3. Argon2id Key Derivation (crypto_pwhash)
    // =========================================================================

    /**
     * @brief Derives a symmetric cryptographic key from a password and salt using Argon2id.
     * 
     * @param password Master password (either as SecureBuffer or std::string).
     * @param salt 16-byte cryptographic salt.
     * @param key_length Output key length in bytes (defaults to 32 bytes for 256-bit keys).
     * @param profile Security/resource profile (default: INTERACTIVE).
     * @return SecureBuffer containing the derived key in locked memory.
     */
    [[nodiscard]] static SecureBuffer deriveKey(
        const SecureBuffer& password,
        const ByteVector& salt,
        size_t key_length = KEY_BYTES,
        Argon2Profile profile = Argon2Profile::INTERACTIVE
    );

    [[nodiscard]] static SecureBuffer deriveKey(
        const std::string& password,
        const ByteVector& salt,
        size_t key_length = KEY_BYTES,
        Argon2Profile profile = Argon2Profile::INTERACTIVE
    );

    [[nodiscard]] static SecureBuffer deriveKey(
        const std::string& password,
        const ByteVector& salt,
        size_t key_length,
        unsigned long long opslimit,
        size_t memlimit
    );

    // Backward-compatible alias
    [[nodiscard]] static SecureBuffer derive_key(
        const std::string& master_password,
        const ByteVector& salt,
        size_t key_length = KEY_BYTES,
        uint32_t iterations = 0 // If 0, uses INTERACTIVE profile
    );

    // =========================================================================
    // 4. Authenticated Encryption with Associated Data (XChaCha20-Poly1305)
    // =========================================================================

    /**
     * @brief Encrypts plaintext using XChaCha20-Poly1305 AEAD with a fresh random nonce.
     * 
     * @param plaintext Sensitive data to encrypt.
     * @param key 256-bit encryption key.
     * @param associated_data Optional authenticated metadata (AAD).
     * @return EncryptedPayload containing nonce, ciphertext, and Poly1305 MAC tag.
     */
    [[nodiscard]] static EncryptedPayload encrypt(
        const SecureBuffer& plaintext,
        const SecureBuffer& key,
        const std::string& associated_data = ""
    );

    [[nodiscard]] static EncryptedPayload encrypt(
        const ByteVector& plaintext,
        const SecureBuffer& key,
        const ByteVector& nonce,
        const ByteVector& associated_data = {}
    );

    /**
     * @brief Decrypts and authenticates an EncryptedPayload using XChaCha20-Poly1305.
     * 
     * Verifies the 128-bit Poly1305 MAC tag before releasing plaintext.
     * 
     * @param payload Struct containing nonce, ciphertext, and tag.
     * @param key 256-bit encryption key.
     * @param associated_data Optional authenticated metadata (must match encryption AAD).
     * @return SecureBuffer containing decrypted plaintext.
     * @throws CryptoException on MAC verification failure or invalid payload.
     */
    [[nodiscard]] static SecureBuffer decrypt(
        const EncryptedPayload& payload,
        const SecureBuffer& key,
        const std::string& associated_data = ""
    );

    [[nodiscard]] static SecureBuffer decrypt(
        const ByteVector& ciphertext,
        const ByteVector& tag,
        const ByteVector& nonce,
        const SecureBuffer& key,
        const ByteVector& associated_data = {}
    );

    // String convenience wrappers
    [[nodiscard]] static EncryptedPayload encrypt_string(
        const std::string& plaintext,
        const SecureBuffer& key,
        const std::string& associated_data = ""
    );

    [[nodiscard]] static std::string decrypt_string(
        const EncryptedPayload& payload,
        const SecureBuffer& key,
        const std::string& associated_data = ""
    );

    // Single-blob combined helpers ([24-byte nonce | ciphertext | 16-byte tag])
    [[nodiscard]] static ByteVector encryptCombined(
        const SecureBuffer& plaintext,
        const SecureBuffer& key,
        const ByteVector& associated_data = {}
    );

    [[nodiscard]] static SecureBuffer decryptCombined(
        const ByteVector& combined_payload,
        const SecureBuffer& key,
        const ByteVector& associated_data = {}
    );

    // =========================================================================
    // 5. Memory Security & Zeroization
    // =========================================================================

    /**
     * @brief Securely clears sensitive memory using libsodium's sodium_memzero().
     * Guaranteed not to be optimized away by compiler dead-code elimination.
     */
    static void clearSensitiveMemory(void* ptr, size_t length) noexcept;
    static void clearSensitiveMemory(std::string& str) noexcept;
    static void clearSensitiveMemory(ByteVector& vec) noexcept;
    static void clearSensitiveMemory(SecureBuffer& buf) noexcept;

    // =========================================================================
    // 6. Cryptographic Hash & Verification Helpers
    // =========================================================================

    /**
     * @brief Computes SHA-256 hash using libsodium's crypto_hash_sha256.
     */
    [[nodiscard]] static std::string sha256_hex(const std::string& input);
    [[nodiscard]] static std::string sha256_hex(const ByteVector& input);
    [[nodiscard]] static ByteVector sha256_bytes(const ByteVector& input);

    /**
     * @brief Performs constant-time memory comparison to protect against timing attacks.
     */
    [[nodiscard]] static bool constant_time_equals(const std::string& a, const std::string& b) noexcept;
    [[nodiscard]] static bool constant_time_equals(const ByteVector& a, const ByteVector& b) noexcept;
    [[nodiscard]] static bool constant_time_equals(const uint8_t* a, const uint8_t* b, size_t len) noexcept;

    // =========================================================================
    // 7. Hex Encoding Helpers
    // =========================================================================
    [[nodiscard]] static std::string to_hex(const ByteVector& bytes);
    [[nodiscard]] static std::string to_hex(const uint8_t* data, size_t length);
    [[nodiscard]] static ByteVector from_hex(const std::string& hex);

private:
    static Argon2Config get_argon2_config(Argon2Profile profile);
};

} // namespace blockvault
