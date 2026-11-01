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
#include <iomanip>
#include <memory>
#include <vector>
#include <string>

using namespace blockvault;

// ANSI Terminal Formatting
#define CYAN    "\033[36m"
#define PURPLE  "\033[35m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"
#define RESET   "\033[0m"

void print_banner() {
    std::cout << PURPLE << BOLD;
    std::cout << R"(
    ██████╗ ██╗      ██████╗  ██████╗██╗  ██╗██╗   ██╗ █████╗ ██╗   ██╗██╗  ████████╗
    ██╔══██╗██║     ██╔═══██╗██╔════╝██║ ██╔╝██║   ██║██╔══██╗██║   ██║██║  ╚══██╔══╝
    ██████╔╝██║     ██║   ██║██║     █████═╝ ██║   ██║███████║██║   ██║██║     ██║   
    ██╔══██╗██║     ██║   ██║██║     ██╔═██╗ ╚██╗ ██╔╝██╔══██║██║   ██║██║     ██║   
    ██████╔╝███████╗╚██████╔╝╚██████╗██║ ╚██╗ ╚████╔╝ ██║  ██║╚██████╔╝███████╗██║   
    ╚═════╝ ╚══════╝ ╚═════╝  ╚═════╝╚═╝  ╚═╝  ╚═══╝  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝   
    )" << RESET << "\n";
    std::cout << CYAN << BOLD << "    Zero-Knowledge Military-Grade Password Management System\n" << RESET;
    std::cout << DIM << "    C++20 Core Architecture | SHA-256 Hash-Linked Audit Ledger | AEAD Ciphers\n\n" << RESET;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    print_banner();

    try {
        std::cout << BOLD << "[1/6] Initializing Persistent Secure Storage Engine..." << RESET << "\n";
        std::string db_file = "vault.db";
        auto db = std::make_shared<DatabaseManager>(db_file);
        std::cout << GREEN << "      -> SQLite database initialized at '" << db_file << "' with WAL mode active." << RESET << "\n\n";

        std::cout << BOLD << "[2/6] Initializing Cryptographic Enclave & Subsystems..." << RESET << "\n";
        auto session_mgr = std::make_shared<SessionManager>(db, 900); // 15-min inactivity timeout
        auto ledger = std::make_shared<AuditLedger>(db);
        AuthManager auth_mgr(db, session_mgr, ledger);
        CredentialManager cred_mgr(db, session_mgr, ledger);
        std::cout << GREEN << "      -> Cryptographic manager, session enclave, and audit ledger online." << RESET << "\n\n";

        // Demo User
        std::string email = "architect@blockvault.sec";
        std::string master_pass = "Quantum#Entropy$Vault99!2026";

        std::cout << BOLD << "[3/6] User Registration & Envelope Key Derivation..." << RESET << "\n";
        User user;
        if (db->get_user_by_email(email).has_value()) {
            std::cout << YELLOW << "      -> User '" << email << "' already registered. Loading profile." << RESET << "\n";
            user = *db->get_user_by_email(email);
        } else {
            user = auth_mgr.register_user(email, master_pass);
            std::cout << GREEN << "      -> Registered user '" << email << "' (UUID: " << user.id() << ")" << RESET << "\n";
            std::cout << DIM << "         Derived KEK via memory-hard KDF. Encrypted random DEK envelope." << RESET << "\n";
        }
        std::cout << "\n";

        std::cout << BOLD << "[4/6] Authenticating User & Unlocking In-Memory Enclave..." << RESET << "\n";
        std::string token = auth_mgr.login(email, master_pass, "127.0.0.1");
        std::cout << GREEN << "      -> Session established! Active Bearer Token:" << RESET << "\n";
        std::cout << CYAN << "         " << token << RESET << "\n";
        std::cout << GREEN << "      -> Memory enclave unlocked with decrypted DEK." << RESET << "\n\n";

        std::cout << BOLD << "[5/6] Demonstrating Authenticated Encryption (AEAD) Credentials..." << RESET << "\n";
        
        // Add Sample Credentials
        cred_mgr.create_credential(
            token, "GitHub Enterprise", "https://github.com", "dineshbabu",
            "ghp_V4ultP@ssw0rd99881177X!", "Developer PAT with full repo and workflow access",
            CredentialCategory::Login, true
        );

        cred_mgr.create_credential(
            token, "AWS Production Root", "https://aws.amazon.com", "root@blockvault.sec",
            "AWS#KMS-256Bit*PolySecret!", "Master cloud encryption key management",
            CredentialCategory::Server, true
        );

        Vault user_vault = cred_mgr.get_vault(token);
        std::cout << GREEN << "      -> Vault populated with " << user_vault.size() << " encrypted credentials." << RESET << "\n";

        for (const auto& cred : user_vault.all()) {
            DecryptedCredential dec = cred_mgr.get_decrypted_credential(token, cred.id());
            std::cout << "         * [" << category_to_string(dec.category) << "] "
                      << BOLD << dec.title << RESET 
                      << " (" << dec.username << ") -> " 
                      << CYAN << dec.password << RESET << "\n";
        }
        std::cout << "\n";

        std::cout << BOLD << "[6/6] Cryptographic Audit Ledger & Chain Integrity Check..." << RESET << "\n";
        IntegrityVerificationResult audit_res = ledger->verify_integrity();
        std::cout << "      -> Total Recorded Security Blocks: " << BOLD << audit_res.total_blocks << RESET << "\n";
        if (audit_res.is_valid) {
            std::cout << GREEN << BOLD << "      -> AUDIT LEDGER INTEGRITY VERIFIED: " << audit_res.error_message << RESET << "\n";
        } else {
            std::cout << RED << BOLD << "      -> TAMPERING DETECTED at block " << audit_res.tampered_index << RESET << "\n";
        }

        std::cout << "\n" << BOLD << "Recent Audit Ledger Blocks:" << RESET << "\n";
        std::vector<Block> records = ledger->get_all_records();
        size_t start_idx = records.size() > 5 ? records.size() - 5 : 0;
        for (size_t i = start_idx; i < records.size(); ++i) {
            const auto& b = records[i];
            std::cout << DIM << "  #" << b.index() << " [" << event_type_to_string(b.event_type()) << "] "
                      << b.details() << " | Hash: " << b.current_hash().substr(0, 16) << "..." << RESET << "\n";
        }

        std::cout << "\n" << GREEN << BOLD << "========================================================\n";
        std::cout << " BlockVault C++20 Core Architecture Running Flawlessly!\n";
        std::cout << "========================================================" << RESET << "\n\n";

        // Lock vault before exit
        auth_mgr.lock_vault(token);
        std::cout << DIM << "Session memory enclave wiped. Vault safely locked." << RESET << "\n\n";

    } catch (const std::exception& e) {
        std::cerr << RED << BOLD << "FATAL ERROR: " << e.what() << RESET << "\n";
        return 1;
    }

    return 0;
}

