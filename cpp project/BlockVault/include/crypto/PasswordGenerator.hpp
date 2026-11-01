#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace blockvault {

enum class PasswordStrengthLevel {
    VeryWeak,
    Weak,
    Fair,
    Strong,
    MilitaryGrade
};

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

struct PasswordOptions {
    size_t length{16};
    bool include_uppercase{true};
    bool include_lowercase{true};
    bool include_numbers{true};
    bool include_symbols{true};
    bool avoid_ambiguous{false}; // Omit 0, O, I, l, 1, etc.
};

struct PasswordAssessment {
    double entropy_bits{0.0};
    PasswordStrengthLevel strength{PasswordStrengthLevel::Weak};
    bool has_uppercase{false};
    bool has_lowercase{false};
    bool has_numbers{false};
    bool has_symbols{false};
    std::vector<std::string> warnings;
    std::vector<std::string> recommendations;
};

/**
 * @class PasswordGenerator
 * @brief Cryptographically secure password generation and entropy analysis engine.
 */
class PasswordGenerator {
public:
    PasswordGenerator() = default;
    ~PasswordGenerator() = default;

    // Non-copyable
    PasswordGenerator(const PasswordGenerator&) = delete;
    PasswordGenerator& operator=(const PasswordGenerator&) = delete;

    // Generate a high-entropy password according to policy
    [[nodiscard]] static std::string generate(const PasswordOptions& options = PasswordOptions{});

    // Assess Shannon entropy and cryptographic strength of any candidate password
    [[nodiscard]] static PasswordAssessment assess(const std::string& password);

    // Calculate exact Shannon entropy: H = L * log2(pool_size)
    [[nodiscard]] static double calculate_entropy(const std::string& password);

    // Generate a 24-word cryptographic emergency recovery mnemonic phrase
    [[nodiscard]] static std::vector<std::string> generate_recovery_phrase();
};

} // namespace blockvault

