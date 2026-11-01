#pragma once

#include <stdexcept>
#include <string>

namespace blockvault {

// Base exception for all BlockVault errors
class BlockVaultException : public std::runtime_error {
public:
    explicit BlockVaultException(const std::string& message) 
        : std::runtime_error(message) {}
};

// Cryptographic failure (e.g. key derivation, decryption authentication failure, corrupted ciphertext)
class CryptoException : public BlockVaultException {
public:
    explicit CryptoException(const std::string& message) 
        : BlockVaultException("Cryptographic Error: " + message) {}
};

// Authentication failure (e.g. invalid master password, expired session, unauthorized access)
class AuthException : public BlockVaultException {
public:
    explicit AuthException(const std::string& message) 
        : BlockVaultException("Authentication Error: " + message) {}
};

// Database failure (e.g. SQL error, constraint violation, connection failure)
class DatabaseException : public BlockVaultException {
public:
    explicit DatabaseException(const std::string& message) 
        : BlockVaultException("Database Error: " + message) {}
};

// Vault operation error (e.g. credential not found, duplicate entry, access denied)
class VaultException : public BlockVaultException {
public:
    explicit VaultException(const std::string& message) 
        : BlockVaultException("Vault Error: " + message) {}
};

// Audit ledger error (e.g. hash chain mismatch, missing previous block)
class AuditException : public BlockVaultException {
public:
    explicit AuditException(const std::string& message) 
        : BlockVaultException("Audit Ledger Error: " + message) {}
};

} // namespace blockvault

