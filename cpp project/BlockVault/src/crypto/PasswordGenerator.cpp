#include "crypto/PasswordGenerator.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"
#include <sodium.h>
#include <cmath>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_set>

namespace blockvault {

// -----------------------------------------------------------------------------
// Character Sets
// -----------------------------------------------------------------------------
static const char* UPPER_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char* LOWER_CHARS = "abcdefghijklmnopqrstuvwxyz";
static const char* NUMBER_CHARS = "0123456789";
static const char* SYMBOL_CHARS = "!@#$%^&*()_+-=[]{}|;:,.<>?";

// Visually ambiguous characters omitted: I, l, 1, O, 0, |
static const char* UPPER_CHARS_NO_AMBIG = "ABCDEFGHJKLMNPQRSTUVWXYZ"; 
static const char* LOWER_CHARS_NO_AMBIG = "abcdefghijkmnpqrstuvwxyz";  
static const char* NUMBER_CHARS_NO_AMBIG = "23456789";                 
static const char* SYMBOL_CHARS_NO_AMBIG = "!@#$%^&*()_+-=[]{};:,.<>?";

// BIP-39 subset of 256 memorable words for emergency recovery phrase generation
static const std::vector<std::string> BIP39_SUBSET = {
    "abandon", "ability", "able", "about", "above", "absent", "absorb", "abstract",
    "absurd", "abuse", "access", "accident", "account", "accuse", "achieve", "acid",
    "acoustic", "acquire", "across", "act", "action", "actor", "actress", "actual",
    "adapt", "add", "addict", "address", "adjust", "admit", "adult", "advance",
    "advice", "aerobic", "affair", "afford", "afraid", "again", "age", "agent",
    "agree", "ahead", "aim", "air", "airport", "aisle", "alarm", "album",
    "alcohol", "alert", "alien", "all", "alley", "allow", "almost", "alone",
    "alpha", "already", "also", "alter", "always", "amateur", "amazing", "among",
    "amount", "amused", "analyst", "anchor", "ancient", "anger", "angle", "angry",
    "animal", "ankle", "announce", "annual", "another", "answer", "antenna", "antique",
    "anxiety", "any", "apart", "apology", "appear", "apple", "approve", "april",
    "arch", "arctic", "area", "arena", "argue", "arm", "armed", "armor",
    "army", "around", "arrange", "arrest", "arrive", "arrow", "art", "artefact",
    "artist", "artwork", "ask", "aspect", "assault", "asset", "assist", "assume",
    "asthma", "athlete", "atom", "attack", "attend", "attitude", "attract", "auction",
    "audit", "august", "aunt", "author", "auto", "autumn", "average", "avocado",
    "avoid", "awake", "aware", "away", "awesome", "awful", "awkward", "axis",
    "baby", "bachelor", "bacon", "badge", "bag", "balance", "balcony", "ball",
    "bamboo", "banana", "banner", "bar", "barely", "bargain", "barrel", "base",
    "basic", "basket", "battle", "beach", "bean", "beauty", "because", "become",
    "beef", "before", "begin", "behave", "behind", "believe", "below", "belt",
    "bench", "benefit", "best", "betray", "better", "between", "beyond", "bicycle",
    "bid", "bike", "bind", "biology", "bird", "birth", "bitter", "black",
    "blade", "blame", "blanket", "blast", "bleak", "bless", "blind", "blood",
    "blossom", "blouse", "blue", "blur", "blush", "board", "boat", "body",
    "boil", "bomb", "bone", "bonus", "book", "boost", "border", "boring",
    "borrow", "boss", "bottom", "bounce", "box", "boy", "bracket", "brain",
    "brand", "brass", "brave", "bread", "breeze", "brick", "bridge", "brief",
    "bright", "bring", "brisk", "broccoli", "broken", "bronze", "broom", "brother",
    "brown", "brush", "bubble", "buddy", "budget", "buffalo", "build", "bulb",
    "bulk", "bullet", "bundle", "bunker", "burden", "burger", "burst", "bus",
    "business", "busy", "butter", "buyer", "buzz", "cabbage", "cabin", "cable"
};

// Cryptographically secure, unbiased random index generator via libsodium
static inline uint32_t secure_unbiased_random(uint32_t range) {
    if (range <= 1) return 0;
    CryptoManager::ensure_initialized();
    return randombytes_uniform(range);
}

// -----------------------------------------------------------------------------
// Options Validation
// -----------------------------------------------------------------------------
void PasswordGenerator::validate_options(const PasswordOptions& options) {
    if (options.length < PasswordOptions::MIN_LENGTH) {
        throw CryptoException(
            "Password length too short: minimum length is " + 
            std::to_string(PasswordOptions::MIN_LENGTH) + " characters"
        );
    }
    if (options.length > PasswordOptions::MAX_LENGTH) {
        throw CryptoException(
            "Password length exceeds permitted limit: maximum is " + 
            std::to_string(PasswordOptions::MAX_LENGTH) + " characters"
        );
    }

    size_t enabled_count = (options.include_uppercase ? 1 : 0) +
                           (options.include_lowercase ? 1 : 0) +
                           (options.include_numbers ? 1 : 0) +
                           (options.include_symbols ? 1 : 0);

    if (enabled_count == 0) {
        throw CryptoException("Invalid password policy: at least one character category must be enabled");
    }
    if (options.length < enabled_count) {
        throw CryptoException(
            "Password length (" + std::to_string(options.length) + 
            ") cannot be smaller than the number of required categories (" + 
            std::to_string(enabled_count) + ")"
        );
    }
}

// -----------------------------------------------------------------------------
// Core Generator
// -----------------------------------------------------------------------------
std::string PasswordGenerator::generate(const PasswordOptions& options) {
    validate_options(options);

    const std::string upper   = options.avoid_ambiguous ? UPPER_CHARS_NO_AMBIG : UPPER_CHARS;
    const std::string lower   = options.avoid_ambiguous ? LOWER_CHARS_NO_AMBIG : LOWER_CHARS;
    const std::string numbers = options.avoid_ambiguous ? NUMBER_CHARS_NO_AMBIG : NUMBER_CHARS;
    const std::string symbols = options.avoid_ambiguous ? SYMBOL_CHARS_NO_AMBIG : SYMBOL_CHARS;

    std::string pool;
    std::string required_prefix;

    // Guarantee that every requested category is represented
    if (options.include_uppercase) {
        pool += upper;
        uint32_t idx = secure_unbiased_random(static_cast<uint32_t>(upper.size()));
        required_prefix += upper[idx];
    }
    if (options.include_lowercase) {
        pool += lower;
        uint32_t idx = secure_unbiased_random(static_cast<uint32_t>(lower.size()));
        required_prefix += lower[idx];
    }
    if (options.include_numbers) {
        pool += numbers;
        uint32_t idx = secure_unbiased_random(static_cast<uint32_t>(numbers.size()));
        required_prefix += numbers[idx];
    }
    if (options.include_symbols) {
        pool += symbols;
        uint32_t idx = secure_unbiased_random(static_cast<uint32_t>(symbols.size()));
        required_prefix += symbols[idx];
    }

    std::string result = required_prefix;
    size_t remaining_chars = options.length - required_prefix.size();

    // Fill the remainder from the combined character pool
    for (size_t i = 0; i < remaining_chars; ++i) {
        uint32_t idx = secure_unbiased_random(static_cast<uint32_t>(pool.size()));
        result += pool[idx];
    }

    // Cryptographic Fisher-Yates Shuffle
    // Eliminates any positional bias where required category characters would always be at the start
    for (size_t i = result.size() - 1; i > 0; --i) {
        uint32_t j = secure_unbiased_random(static_cast<uint32_t>(i + 1));
        std::swap(result[i], result[j]);
    }

    return result;
}

SecureBuffer PasswordGenerator::generate_secure(const PasswordOptions& options) {
    std::string plain = generate(options);
    SecureBuffer secure_buf(plain);
    CryptoManager::clearSensitiveMemory(plain);
    return secure_buf;
}

// -----------------------------------------------------------------------------
// Shannon Entropy Calculation: H = L * log2(PoolSize)
// -----------------------------------------------------------------------------
double PasswordGenerator::calculate_entropy(const std::string& password) {
    if (password.empty()) return 0.0;

    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_symbol = false;

    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) has_upper = true;
        else if (std::islower(static_cast<unsigned char>(c))) has_lower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) has_digit = true;
        else has_symbol = true;
    }

    double pool_size = 0.0;
    if (has_lower) pool_size += 26.0;
    if (has_upper) pool_size += 26.0;
    if (has_digit) pool_size += 10.0;
    if (has_symbol) pool_size += 32.0;

    if (pool_size <= 0.0) return 0.0;

    // Shannon Information Entropy formula: H = L * log2(N)
    return static_cast<double>(password.size()) * (std::log(pool_size) / std::log(2.0));
}

// -----------------------------------------------------------------------------
// Crack Time Estimation Display
// -----------------------------------------------------------------------------
std::string PasswordGenerator::estimate_crack_time_display(double entropy_bits) {
    if (entropy_bits <= 0.0) return "Instant";

    // Assume high-performance offline cluster doing 10 billion guesses/second (10^10 hashes/sec)
    constexpr double GUESSES_PER_SEC = 1.0e10;
    
    // Average time to find password is 2^(H - 1) / Rate
    double log2_seconds = (entropy_bits - 1.0) - (std::log(GUESSES_PER_SEC) / std::log(2.0));
    if (log2_seconds <= 0.0) {
        return "Instant (< 1 second)";
    }

    double seconds = std::pow(2.0, log2_seconds);

    constexpr double MINUTE = 60.0;
    constexpr double HOUR   = 3600.0;
    constexpr double DAY    = 86400.0;
    constexpr double YEAR   = 31536000.0;
    constexpr double CENTURY = YEAR * 100.0;

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);

    if (seconds < MINUTE) {
        ss << seconds << " seconds";
    } else if (seconds < HOUR) {
        ss << (seconds / MINUTE) << " minutes";
    } else if (seconds < DAY) {
        ss << (seconds / HOUR) << " hours";
    } else if (seconds < YEAR) {
        ss << (seconds / DAY) << " days";
    } else if (seconds < CENTURY) {
        ss << (seconds / YEAR) << " years";
    } else if (seconds < CENTURY * 1000.0) {
        ss << (seconds / CENTURY) << " centuries";
    } else {
        ss << "> 1,000 Centuries (Virtually Unbreakable)";
    }

    return ss.str();
}

// -----------------------------------------------------------------------------
// Comprehensive Password Assessment
// -----------------------------------------------------------------------------
PasswordAssessment PasswordGenerator::assess(const std::string& password) {
    PasswordAssessment assessment;
    assessment.entropy_bits = calculate_entropy(password);

    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) assessment.has_uppercase = true;
        else if (std::islower(static_cast<unsigned char>(c))) assessment.has_lowercase = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) assessment.has_numbers = true;
        else assessment.has_symbols = true;
    }

    // Determine character pool size
    assessment.pool_size = 0;
    if (assessment.has_lowercase) assessment.pool_size += 26;
    if (assessment.has_uppercase) assessment.pool_size += 26;
    if (assessment.has_numbers) assessment.pool_size += 10;
    if (assessment.has_symbols) assessment.pool_size += 32;

    // Detect repeated characters (e.g. 'aaa')
    if (password.size() >= 3) {
        for (size_t i = 0; i + 2 < password.size(); ++i) {
            if (password[i] == password[i + 1] && password[i] == password[i + 2]) {
                assessment.has_repeated_sequences = true;
                break;
            }
        }
    }

    // Warnings and Recommendations
    if (password.size() < 8) {
        assessment.warnings.push_back("Password is shorter than the recommended minimum of 8 characters.");
    }
    if (assessment.has_repeated_sequences) {
        assessment.warnings.push_back("Contains repeated consecutive characters (e.g. 'aaa').");
    }
    if (!assessment.has_uppercase) {
        assessment.recommendations.push_back("Include uppercase letters (A-Z) to expand character space.");
    }
    if (!assessment.has_numbers) {
        assessment.recommendations.push_back("Include numeric digits (0-9) to defend against dictionary attacks.");
    }
    if (!assessment.has_symbols) {
        assessment.recommendations.push_back("Include special symbols (!@#$) to maximize mathematical entropy.");
    }

    // Qualitative Strength Classification
    if (assessment.entropy_bits < 36.0) {
        assessment.strength = PasswordStrengthLevel::VeryWeak;
        assessment.score = static_cast<uint32_t>(std::min(25.0, assessment.entropy_bits * 0.7));
    } else if (assessment.entropy_bits < 53.0) {
        assessment.strength = PasswordStrengthLevel::Weak;
        assessment.score = static_cast<uint32_t>(25 + (assessment.entropy_bits - 36.0) * 1.5);
    } else if (assessment.entropy_bits < 65.0) {
        assessment.strength = PasswordStrengthLevel::Fair;
        assessment.score = static_cast<uint32_t>(50 + (assessment.entropy_bits - 53.0) * 1.6);
    } else if (assessment.entropy_bits < 85.0) {
        assessment.strength = PasswordStrengthLevel::Strong;
        assessment.score = static_cast<uint32_t>(70 + (assessment.entropy_bits - 65.0) * 1.25);
    } else {
        assessment.strength = PasswordStrengthLevel::MilitaryGrade;
        assessment.score = static_cast<uint32_t>(std::min(100.0, 95.0 + (assessment.entropy_bits - 85.0) * 0.25));
    }

    if (assessment.has_repeated_sequences && assessment.score > 15) {
        assessment.score -= 15;
    }

    assessment.crack_time_display = estimate_crack_time_display(assessment.entropy_bits);

    return assessment;
}

// -----------------------------------------------------------------------------
// Emergency Recovery Phrase Generation (BIP-39 Mnemonic)
// -----------------------------------------------------------------------------
std::vector<std::string> PasswordGenerator::generate_recovery_phrase(size_t word_count) {
    if (word_count == 0) return {};
    
    std::vector<std::string> phrase;
    phrase.reserve(word_count);

    for (size_t i = 0; i < word_count; ++i) {
        uint32_t idx = secure_unbiased_random(static_cast<uint32_t>(BIP39_SUBSET.size()));
        phrase.push_back(BIP39_SUBSET[idx]);
    }

    return phrase;
}

} // namespace blockvault
