# BlockVault – Zero-Knowledge Password Management System

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Compiler](https://img.shields.io/badge/GCC-14.2%20%7C%20Clang%2019.1-brightgreen.svg)](https://winlibs.com/)
[![Database](https://img.shields.io/badge/SQLite3-WAL%20Mode-blue.svg)](https://www.sqlite.org/)
[![Security](https://img.shields.io/badge/Architecture-Zero--Knowledge-purple.svg)](docs/CLASS_SPECIFICATIONS.md)
[![Tests](https://img.shields.io/badge/Test%20Suite-6%20Passed%2C%200%20Failed-success.svg)](tests/)

---

## Overview

**BlockVault** is an enterprise-grade, zero-knowledge password management system engineered in **modern C++20**. It implements defense-in-depth security principles:

1. **Zero-Knowledge Envelope Encryption:** The master password derives a Key Encryption Key (KEK) using a memory-hard KDF, which in turn unwraps a randomly generated Data Encryption Key (DEK). The server/storage layer never stores or encounters plaintext passwords.
2. **Volatile Memory Enclave (`SecureBuffer`):** Sensitive cryptographic keys are managed with strict RAII, prevented from being swapped to disk, and wiped via volatile memory loops upon destructor execution.
3. **Local Hash-Linked Audit Ledger:** Every security transaction (registration, login, credential read/write, vault locking) is recorded in an immutable, append-only SHA-256 hash chain with instant tamper-detection capabilities.
4. **SQLite Persistence with WAL:** Embedded relational persistence using SQLite3 configured with Write-Ahead Logging (WAL) and prepared statement parameter binding.

---

## Project Structure

```
BlockVault/
├── include/                   # Public Header Files (C++20)
│   ├── common/
│   │   ├── Exceptions.hpp     # Structured exception hierarchy
│   │   └── Types.hpp          # Domain enums, timestamps, type aliases
│   ├── models/
│   │   ├── User.hpp           # User identity & cryptographic envelope
│   │   ├── Credential.hpp     # Encrypted credential model
│   │   ├── Block.hpp          # Hash-linked audit ledger block
│   │   └── Vault.hpp          # In-memory domain container & queries
│   ├── crypto/
│   │   ├── SecureBuffer.hpp   # RAII protected memory container
│   │   ├── CryptoManager.hpp  # CSPRNG, SHA-256, PBKDF2, AEAD, constant-time
│   │   └── PasswordGenerator.hpp # Entropy analysis & 24-word recovery phrases
│   ├── db/
│   │   └── DatabaseManager.hpp# SQLite3 WAL engine & prepared statements
│   ├── ledger/
│   │   └── AuditLedger.hpp    # SHA-256 hash chaining & verification engine
│   ├── auth/
│   │   ├── SessionManager.hpp # Active session enclave & timeout policy
│   │   └── AuthManager.hpp    # Zero-knowledge authentication service
│   └── vault/
│       └── CredentialManager.hpp # Tenant-isolated credential CRUD service
├── src/                       # Implementation Files
│   ├── crypto/
│   ├── models/
│   ├── db/
│   ├── ledger/
│   ├── auth/
│   ├── vault/
│   └── main.cpp               # Showcase CLI application
├── third_party/
│   └── sqlite/                # Official SQLite3 amalgamation (3.53+)
├── tests/
│   ├── main_test.cpp          # Comprehensive unit test suite
│   └── test_runner.exe        # Standalone compiled test runner
├── docs/
│   └── CLASS_SPECIFICATIONS.md# Detailed responsibilities & API documentation
├── bin/
│   └── blockvault.exe         # Standalone compiled executable
├── build.ps1                  # PowerShell script to compile & run test suite
├── build_app.ps1              # PowerShell script to build standalone binary
├── ARCHITECTURE.md            # System architecture specification
├── API_SPEC.md                # REST API endpoints contract
├── THREAT_MODEL.md            # STRIDE threat model & mitigations
└── ROADMAP.md                 # 6-Phase development roadmap
```

---

## Building and Running

### Prerequisites
- Modern C++20 compiler: **GCC 14.2+** or **Clang 19.1+** (included in WinLibs MinGW-w64).
- PowerShell (Windows) or Bash (Linux/macOS).

### 1. Run the Automated Test Suite
```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```
**Output:**
```
========================================================
 BLOCKVAULT C++20 CORE ARCHITECTURE TEST SUITE
========================================================

[TEST] SecureBuffer Allocation, Move & Zeroization ... PASSED
[TEST] CryptoManager SHA-256, CSPRNG & AEAD Roundtrip ... PASSED
[TEST] PasswordGenerator Policy, Shannon Entropy & 24-Word Recovery ... PASSED
[TEST] DatabaseManager SQLite Schema & Entity Persistence ... PASSED
[TEST] AuditLedger Hash Chaining & Cryptographic Tamper Detection ... PASSED
[TEST] End-to-End Zero-Knowledge Auth & Credential CRUD ... PASSED

--------------------------------------------------------
 RESULTS: 6 Passed, 0 Failed
========================================================
```

### 2. Build and Launch the Showcase Application
```powershell
powershell -ExecutionPolicy Bypass -File .\build_app.ps1
```

---

## Technical Documentation
For complete member-by-member class specifications, data member descriptions, and usage examples, refer to [`docs/CLASS_SPECIFICATIONS.md`](docs/CLASS_SPECIFICATIONS.md).
