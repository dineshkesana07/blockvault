#pragma once

#include "crypto/SecureBuffer.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace blockvault {

/**
 * @enum PasswordStrengthLevel
 * @brief Qualitative categorization of password security based on Shannon entropy.
 */
enum class PasswordStrengthLevel {
    VeryWeak,     ///< < 36 bits of entropy (easily crackable in seconds)
    Weak,         ///< 36 - 52 bits of entropy (vulnerable to fast GPU clusters)
    Fair,         ///< 53 - 64 bits of entropy (moderate resistance against targeted attacks)
    Strong,       ///< 65 - 84 bits of entropy (resistant to massive online/offline dictionary attacks)
    MilitaryGrade ///< >= 85 bits of entropy (mathematically impossible to brute force)
};

/**
 * @brief Converts PasswordStrengthLevel to standard uppercase string.
 */
inline std::string strength_level_to_string(PasswordStrengthLevel level) {
    switch (level) {
        case PasswordStrengthLevel::VeryWeak: return "VERY_WEAK";
        case PasswordStrengthLevel::Weak: return "WEAK";
        case PasswordStrengthLevel::Fair: return "FAIR";
        case PasswordStrengthLevel::Strong: return "STRONG";
        case PasswordStrengthLevel::MilitaryGrade: return "MILITARY_GRADE";
        default: return "WEAK";
    }
}

/**
 * @struct PasswordOptions
 * @brief Configurable parameters governing password generation.
 */
struct PasswordOptions {
    size_t length{16};             ///< Total length of the generated password (minimum 4, maximum 1024)
    bool include_uppercase{true};   ///< Include uppercase letters (A-Z)
    bool include_lowercase{true};   ///< Include lowercase letters (a-z)
    bool include_numbers{true};     ///< Include numeric digits (0-9)
    bool include_symbols{true};     ///< Include special punctuation characters (!@#$%...)
    bool avoid_ambiguous{false};    ///< Exclude visually ambiguous characters (0, O, I, l, 1, etc.)
    
    // Bounds constraints
    static constexpr size_t MIN_LENGTH = 4;
    static constexpr size_t MAX_LENGTH = 1024;
};

/**
 * @struct PasswordAssessment
 * @brief Comprehensive mathematical and heuristic security assessment of a candidate password.
 */
struct PasswordAssessment {
    double entropy_bits{0.0};                     ///< Shannon information entropy in bits (H = L * log2(N))
    PasswordStrengthLevel strength{PasswordStrengthLevel::Weak}; ///< Strength classification tier
    uint32_t score{0};                            ///< Normalized quality score from 0 (terrible) to 100 (flawless)
    size_t pool_size{0};                          ///< Effective character set pool size (N)
    std::string crack_time_display;               ///< Human-readable estimated crack time (e.g. "3.2 Centuries")
    bool has_uppercase{false};                    ///< Contains at least one uppercase letter
    bool has_lowercase{false};                    ///< Contains at least one lowercase letter
    bool has_numbers{false};                      ///< Contains at least one numeric digit
    bool has_symbols{false};                      ///< Contains at least one special symbol
    bool has_repeated_sequences{false};           ///< Contains duplicate or sequential runs (e.g. 'aaaa', '1234')
    std::vector<std::string> warnings;           ///< Actionable security warnings
    std::vector<std::string> recommendations;    ///< Improvement suggestions
};

/**
 * @class PasswordGenerator
 * @brief Cryptographically secure password generation and entropy analysis engine.
 * 
 * Utilizes libsodium's CSPRNG (randombytes_uniform) to provide unbiased,
 * modulo-free random selection from validated character pools. Guarantees
 * that every enabled character category is represented in the output.
 */
class PasswordGenerator {
public:
    PasswordGenerator() = delete;
    ~PasswordGenerator() = delete;

    // Non-copyable & Non-movable static utility class
    PasswordGenerator(const PasswordGenerator&) = delete;
    PasswordGenerator& operator=(const PasswordGenerator&) = delete;

    /**
     * @brief Generates a cryptographically secure random password matching policy options.
     * 
     * Uses rejection sampling via libsodium to prevent modulo bias.
     * Guarantees that at least one character from each enabled category is included.
     * Applies a cryptographic Fisher-Yates shuffle to prevent predictable character placement.
     * 
     * @param options Policy configuration parameters.
     * @return std::string The generated password.
     * @throws CryptoException if options are invalid (e.g. length < enabled categories).
     */
    [[nodiscard]] static std::string generate(const PasswordOptions& options = PasswordOptions{});

    /**
     * @brief Generates a password directly into a SecureBuffer to avoid heap string copies.
     * The resulting buffer is pinned in RAM and protected against disk swapping.
     */
    [[nodiscard]] static SecureBuffer generate_secure(const PasswordOptions& options = PasswordOptions{});

    /**
     * @brief Validates that options are mathematically sound and satisfiable.
     * @throws CryptoException if options are invalid.
     */
    static void validate_options(const PasswordOptions& options);

    /**
     * @brief Calculates exact Shannon entropy in bits for a password:
     *        H = L * log2(PoolSize)
     * 
     * @param password Candidate password string.
     * @return double Entropy in bits.
     */
    [[nodiscard]] static double calculate_entropy(const std::string& password);

    /**
     * @brief Performs a full heuristic and mathematical assessment of a password.
     * Detects character set coverage, Shannon entropy, repeated sequences, and calculates crack times.
     * 
     * @param password Candidate password string.
     * @return PasswordAssessment Struct containing entropy, score, strength level, and feedback.
     */
    [[nodiscard]] static PasswordAssessment assess(const std::string& password);

    /**
     * @brief Generates an emergency recovery mnemonic phrase from a BIP-39 wordlist.
     * 
     * @param word_count Number of words to generate (defaults to 24 words = 256 bits entropy).
     * @return std::vector<std::string> List of mnemonic words.
     */
    [[nodiscard]] static std::vector<std::string> generate_recovery_phrase(size_t word_count = 24);

    /**
     * @brief Returns a human-friendly estimated crack time based on entropy.
     * Assumes an offline cracking rig capable of 10 billion guesses/second (10^10 hashes/sec).
     */
    [[nodiscard]] static std::string estimate_crack_time_display(double entropy_bits);
};

} // namespace blockvault
