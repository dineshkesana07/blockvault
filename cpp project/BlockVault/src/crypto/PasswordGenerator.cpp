#include "crypto/PasswordGenerator.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"
#include <sodium.h>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <unordered_set>

namespace blockvault {

static const char* UPPER_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char* LOWER_CHARS = "abcdefghijklmnopqrstuvwxyz";
static const char* NUMBER_CHARS = "0123456789";
static const char* SYMBOL_CHARS = "!@#$%^&*()_+-=[]{}|;:,.<>?";

static const char* UPPER_CHARS_NO_AMBIG = "ABCDEFGHJKLMNPQRSTUVWXYZ"; // removed I, O
static const char* LOWER_CHARS_NO_AMBIG = "abcdefghijkmnpqrstuvwxyz";  // removed l, o
static const char* NUMBER_CHARS_NO_AMBIG = "23456789";                 // removed 0, 1
static const char* SYMBOL_CHARS_NO_AMBIG = "!@#$%^&*()_+-=[]{};:,.<>?";

// BIP-39 subset of 256 memorable words for 24-word emergency recovery phrase
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

static uint32_t secure_unbiased_random(uint32_t range) {
    if (range <= 1) return 0;
    CryptoManager::ensure_initialized();
    return randombytes_uniform(range);
}

std::string PasswordGenerator::generate(const PasswordOptions& options) {
    if (options.length < 4) {
        throw CryptoException("Password length must be at least 4 characters");
    }

    std::string pool;
    std::string required_chars;

    const std::string upper = options.avoid_ambiguous ? UPPER_CHARS_NO_AMBIG : UPPER_CHARS;
    const std::string lower = options.avoid_ambiguous ? LOWER_CHARS_NO_AMBIG : LOWER_CHARS;
    const std::string numbers = options.avoid_ambiguous ? NUMBER_CHARS_NO_AMBIG : NUMBER_CHARS;
    const std::string symbols = options.avoid_ambiguous ? SYMBOL_CHARS_NO_AMBIG : SYMBOL_CHARS;

    if (options.include_uppercase) {
        pool += upper;
        required_chars += upper[secure_unbiased_random(static_cast<uint32_t>(upper.size()))];
    }
    if (options.include_lowercase) {
        pool += lower;
        required_chars += lower[secure_unbiased_random(static_cast<uint32_t>(lower.size()))];
    }
    if (options.include_numbers) {
        pool += numbers;
        required_chars += numbers[secure_unbiased_random(static_cast<uint32_t>(numbers.size()))];
    }
    if (options.include_symbols) {
        pool += symbols;
        required_chars += symbols[secure_unbiased_random(static_cast<uint32_t>(symbols.size()))];
    }

    if (pool.empty()) {
        throw CryptoException("At least one character set must be enabled");
    }

    std::string result = required_chars;
    while (result.size() < options.length) {
        uint32_t idx = secure_unbiased_random(static_cast<uint32_t>(pool.size()));
        result += pool[idx];
    }

    // Cryptographic Fisher-Yates Shuffle
    for (size_t i = result.size() - 1; i > 0; --i) {
        uint32_t j = secure_unbiased_random(static_cast<uint32_t>(i + 1));
        std::swap(result[i], result[j]);
    }

    return result;
}

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

    // H = L * log2(pool_size)
    return static_cast<double>(password.size()) * (std::log(pool_size) / std::log(2.0));
}

PasswordAssessment PasswordGenerator::assess(const std::string& password) {
    PasswordAssessment assessment;
    assessment.entropy_bits = calculate_entropy(password);

    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) assessment.has_uppercase = true;
        else if (std::islower(static_cast<unsigned char>(c))) assessment.has_lowercase = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) assessment.has_numbers = true;
        else assessment.has_symbols = true;
    }

    if (password.size() < 8) {
        assessment.warnings.push_back("Password is shorter than the recommended minimum of 8 characters.");
    }
    if (!assessment.has_uppercase) {
        assessment.recommendations.push_back("Add uppercase letters to increase character space.");
    }
    if (!assessment.has_numbers) {
        assessment.recommendations.push_back("Add digits to defend against dictionary attacks.");
    }
    if (!assessment.has_symbols) {
        assessment.recommendations.push_back("Add special symbols to maximize Shannon entropy.");
    }

    if (assessment.entropy_bits < 32.0) {
        assessment.strength = PasswordStrengthLevel::VeryWeak;
    } else if (assessment.entropy_bits < 50.0) {
        assessment.strength = PasswordStrengthLevel::Weak;
    } else if (assessment.entropy_bits < 75.0) {
        assessment.strength = PasswordStrengthLevel::Fair;
    } else if (assessment.entropy_bits < 100.0) {
        assessment.strength = PasswordStrengthLevel::Strong;
    } else {
        assessment.strength = PasswordStrengthLevel::MilitaryGrade;
    }

    return assessment;
}

std::vector<std::string> PasswordGenerator::generate_recovery_phrase() {
    std::vector<std::string> phrase;
    phrase.reserve(24);
    for (size_t i = 0; i < 24; ++i) {
        uint32_t idx = secure_unbiased_random(static_cast<uint32_t>(BIP39_SUBSET.size()));
        phrase.push_back(BIP39_SUBSET[idx]);
    }
    return phrase;
}

} // namespace blockvault

