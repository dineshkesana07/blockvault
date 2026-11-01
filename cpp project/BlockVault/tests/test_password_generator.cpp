#include "crypto/PasswordGenerator.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <cctype>

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
        std::cout << "[PASSWORD TEST] " << #test_func << " ... "; \
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

// Helper character categorization lambdas
static bool is_symbol(char c) {
    std::string symbols = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    return symbols.find(c) != std::string::npos;
}

// =============================================================================
// Test 1: Minimum and Maximum Length Boundaries
// =============================================================================
void test_length_boundaries() {
    CryptoManager::initialize();

    // 1. Minimum valid length (4 chars, 4 categories)
    PasswordOptions opt_min;
    opt_min.length = 4;
    opt_min.include_uppercase = true;
    opt_min.include_lowercase = true;
    opt_min.include_numbers = true;
    opt_min.include_symbols = true;
    std::string pwd_min = PasswordGenerator::generate(opt_min);
    assert(pwd_min.length() == 4);

    // 2. Length below minimum (3, 2, 1, 0) must throw CryptoException
    for (size_t len = 0; len < 4; ++len) {
        PasswordOptions opt_invalid = opt_min;
        opt_invalid.length = len;
        bool threw = false;
        try {
            (void)PasswordGenerator::generate(opt_invalid);
        } catch (const CryptoException&) {
            threw = true;
        }
        assert(threw == true);
    }

    // 3. Length smaller than number of active categories must throw
    PasswordOptions opt_too_short;
    opt_too_short.length = 4;
    // With 4 categories, length 4 works, but length 3 must throw
    opt_too_short.length = 3;
    bool mismatch_threw = false;
    try {
        (void)PasswordGenerator::generate(opt_too_short);
    } catch (const CryptoException&) {
        mismatch_threw = true;
    }
    assert(mismatch_threw == true);

    // 4. Standard diverse lengths
    std::vector<size_t> test_lengths = { 8, 16, 24, 32, 64, 128, 256, 512 };
    for (size_t len : test_lengths) {
        PasswordOptions opt;
        opt.length = len;
        std::string p = PasswordGenerator::generate(opt);
        assert(p.length() == len);
    }

    // 5. Maximum permitted length (1024)
    PasswordOptions opt_max;
    opt_max.length = 1024;
    std::string pwd_max = PasswordGenerator::generate(opt_max);
    assert(pwd_max.length() == 1024);

    // 6. Above maximum length (1025) must throw CryptoException
    PasswordOptions opt_over_max;
    opt_over_max.length = 1025;
    bool over_max_threw = false;
    try {
        (void)PasswordGenerator::generate(opt_over_max);
    } catch (const CryptoException&) {
        over_max_threw = true;
    }
    assert(over_max_threw == true);
}

// =============================================================================
// Test 2: All 15 Category Combinations + Validation
// =============================================================================
void test_category_combinations() {
    CryptoManager::initialize();

    // Zero categories enabled must throw
    PasswordOptions zero_opt;
    zero_opt.length = 16;
    zero_opt.include_uppercase = false;
    zero_opt.include_lowercase = false;
    zero_opt.include_numbers = false;
    zero_opt.include_symbols = false;
    bool zero_threw = false;
    try {
        (void)PasswordGenerator::generate(zero_opt);
    } catch (const CryptoException&) {
        zero_threw = true;
    }
    assert(zero_threw == true);

    // Test each combination from 1 to 15
    for (int mask = 1; mask < 16; ++mask) {
        bool up   = (mask & 1) != 0;
        bool low  = (mask & 2) != 0;
        bool num  = (mask & 4) != 0;
        bool sym  = (mask & 8) != 0;

        PasswordOptions opt;
        opt.length = 20;
        opt.include_uppercase = up;
        opt.include_lowercase = low;
        opt.include_numbers = num;
        opt.include_symbols = sym;

        std::string p = PasswordGenerator::generate(opt);
        assert(p.length() == 20);

        bool found_up = false;
        bool found_low = false;
        bool found_num = false;
        bool found_sym = false;

        for (char c : p) {
            if (std::isupper(static_cast<unsigned char>(c))) found_up = true;
            else if (std::islower(static_cast<unsigned char>(c))) found_low = true;
            else if (std::isdigit(static_cast<unsigned char>(c))) found_num = true;
            else if (is_symbol(c)) found_sym = true;
        }

        // Verify that requested categories ARE present
        if (up) assert(found_up == true);
        if (low) assert(found_low == true);
        if (num) assert(found_num == true);
        if (sym) assert(found_sym == true);

        // Verify that NON-requested categories are NOT present
        if (!up) assert(found_up == false);
        if (!low) assert(found_low == false);
        if (!num) assert(found_num == false);
        if (!sym) assert(found_sym == false);
    }
}

// =============================================================================
// Test 3: Representation Guarantee in 100 Consecutive Generations
// =============================================================================
void test_category_representation_guarantee() {
    CryptoManager::initialize();

    PasswordOptions opt;
    opt.length = 8; // Small length to rigorously test representation
    opt.include_uppercase = true;
    opt.include_lowercase = true;
    opt.include_numbers = true;
    opt.include_symbols = true;

    for (int i = 0; i < 100; ++i) {
        std::string p = PasswordGenerator::generate(opt);
        assert(p.length() == 8);

        bool has_u = false, has_l = false, has_n = false, has_s = false;
        for (char c : p) {
            if (std::isupper(static_cast<unsigned char>(c))) has_u = true;
            else if (std::islower(static_cast<unsigned char>(c))) has_l = true;
            else if (std::isdigit(static_cast<unsigned char>(c))) has_n = true;
            else if (is_symbol(c)) has_s = true;
        }
        assert(has_u && has_l && has_n && has_s);
    }
}

// =============================================================================
// Test 4: Ambiguous Character Filtering (avoid_ambiguous)
// =============================================================================
void test_avoid_ambiguous_characters() {
    CryptoManager::initialize();

    PasswordOptions opt;
    opt.length = 64;
    opt.include_uppercase = true;
    opt.include_lowercase = true;
    opt.include_numbers = true;
    opt.include_symbols = true;
    opt.avoid_ambiguous = true; // Excludes: 0, O, I, l, 1, |

    std::string ambiguous_set = "0OIl1|";

    for (int i = 0; i < 50; ++i) {
        std::string p = PasswordGenerator::generate(opt);
        for (char c : p) {
            assert(ambiguous_set.find(c) == std::string::npos);
        }
    }
}

// =============================================================================
// Test 5: Fisher-Yates Positional Unbiasedness
// =============================================================================
void test_positional_randomness() {
    CryptoManager::initialize();

    // Verify that index 0 is not always of the same character category
    PasswordOptions opt;
    opt.length = 16;
    opt.include_uppercase = true;
    opt.include_lowercase = true;
    opt.include_numbers = true;
    opt.include_symbols = true;

    std::set<char> first_chars;
    for (int i = 0; i < 100; ++i) {
        std::string p = PasswordGenerator::generate(opt);
        first_chars.insert(p[0]);
    }

    // Over 100 random generations, first character should have rich diversity (>15 distinct characters)
    assert(first_chars.size() >= 15);
}

// =============================================================================
// Test 6: Shannon Entropy & Password Assessment
// =============================================================================
void test_shannon_entropy_and_assessment() {
    // 1. Empty string
    assert(PasswordGenerator::calculate_entropy("") == 0.0);

    // 2. Weak single-category password: "password"
    // Pool = 26 (lowercase only), L = 8 => H = 8 * log2(26) ≈ 8 * 4.7004 = 37.6 bits
    double ent_weak = PasswordGenerator::calculate_entropy("password");
    assert(ent_weak > 35.0 && ent_weak < 40.0);
    PasswordAssessment assess_weak = PasswordGenerator::assess("password");
    assert(assess_weak.strength == PasswordStrengthLevel::Weak);
    assert(!assess_weak.has_uppercase);
    assert(!assess_weak.has_numbers);
    assert(!assess_weak.has_symbols);

    // 3. Repeated sequence detection: "aaaabbbb1111"
    PasswordAssessment assess_rep = PasswordGenerator::assess("aaaabbbb1111");
    assert(assess_rep.has_repeated_sequences == true);

    // 4. Strong enterprise password: 16 chars with all 4 categories
    // Pool = 26 + 26 + 10 + 32 = 94, L = 16 => H = 16 * log2(94) ≈ 16 * 6.5546 = 104.87 bits
    std::string strong_pwd = "V4ult#K3y*2026!Xz";
    double ent_strong = PasswordGenerator::calculate_entropy(strong_pwd);
    assert(ent_strong > 100.0);
    PasswordAssessment assess_strong = PasswordGenerator::assess(strong_pwd);
    assert(assess_strong.strength == PasswordStrengthLevel::MilitaryGrade);
    assert(assess_strong.has_uppercase);
    assert(assess_strong.has_lowercase);
    assert(assess_strong.has_numbers);
    assert(assess_strong.has_symbols);
    assert(assess_strong.score >= 95);
    assert(assess_strong.crack_time_display.find("Centuries") != std::string::npos);
}

// =============================================================================
// Test 7: SecureBuffer Generation & Memory Protection
// =============================================================================
void test_secure_buffer_generation() {
    CryptoManager::initialize();

    PasswordOptions opt;
    opt.length = 32;
    SecureBuffer sec_pwd = PasswordGenerator::generate_secure(opt);
    assert(sec_pwd.size() == 32);
    assert(!sec_pwd.empty());

    // Verify string conversion
    std::string str_pwd = sec_pwd.to_string();
    assert(str_pwd.length() == 32);

    // Zeroize buffer
    sec_pwd.zeroize();
    for (size_t i = 0; i < sec_pwd.size(); ++i) {
        assert(sec_pwd[i] == 0);
    }
}

// =============================================================================
// Test 8: BIP-39 Emergency Recovery Phrase Generation
// =============================================================================
void test_recovery_phrase() {
    CryptoManager::initialize();

    // 24-word default recovery phrase
    std::vector<std::string> phrase24 = PasswordGenerator::generate_recovery_phrase(24);
    assert(phrase24.size() == 24);
    for (const auto& word : phrase24) {
        assert(!word.empty());
        assert(word.length() >= 3);
    }

    // 12-word recovery phrase
    std::vector<std::string> phrase12 = PasswordGenerator::generate_recovery_phrase(12);
    assert(phrase12.size() == 12);
}

// =============================================================================
// Main Test Runner Entrypoint
// =============================================================================
int main() {
    std::cout << "\n========================================================\n";
    std::cout << " BLOCKVAULT C++20 PASSWORD GENERATOR TEST SUITE\n";
    std::cout << "========================================================\n\n";

    RUN_TEST(test_length_boundaries);
    RUN_TEST(test_category_combinations);
    RUN_TEST(test_category_representation_guarantee);
    RUN_TEST(test_avoid_ambiguous_characters);
    RUN_TEST(test_positional_randomness);
    RUN_TEST(test_shannon_entropy_and_assessment);
    RUN_TEST(test_secure_buffer_generation);
    RUN_TEST(test_recovery_phrase);

    std::cout << "\n--------------------------------------------------------\n";
    std::cout << " PASSWORD GENERATOR RESULTS: " 
              << g_tests_passed << " Passed, " 
              << g_tests_failed << " Failed\n";
    std::cout << "========================================================\n\n";

    return (g_tests_failed == 0) ? 0 : 1;
}

