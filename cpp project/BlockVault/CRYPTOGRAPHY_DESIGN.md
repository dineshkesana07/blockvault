# BlockVault – Cryptography Architecture & Engineering Specification

> **Document Classification:** Enterprise Cryptographic Architecture Specification  
> **Target Library:** [libsodium 1.0.22](https://github.com/jedisct1/libsodium) (High-assurance, auditable cryptographic library)  
> **Primary Algorithms:** Argon2id (v1.3), XChaCha20-Poly1305 AEAD, SHA-256  
> **Language Standard:** C++20 (ISO/IEC 14882:2020)  
> **Source Modules:** [`include/crypto/CryptoManager.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/crypto/CryptoManager.hpp), [`src/crypto/CryptoManager.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/crypto/CryptoManager.cpp), [`include/crypto/SecureBuffer.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/crypto/SecureBuffer.hpp), [`src/crypto/SecureBuffer.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/crypto/SecureBuffer.cpp), [`tests/test_crypto.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/tests/test_crypto.cpp)

---

## Table of Contents
1. [Cryptographic Philosophy & Core Tenets](#1-cryptographic-philosophy--core-tenets)
2. [Beginner-Friendly Cryptography Primer](#2-beginner-friendly-cryptography-primer)
   - [2.1 Plaintext vs. Ciphertext](#21-plaintext-vs-ciphertext)
   - [2.2 Cryptographic Key](#22-cryptographic-key)
   - [2.3 Cryptographic Salt](#23-cryptographic-salt)
   - [2.4 Cryptographic Nonce](#24-cryptographic-nonce)
   - [2.5 Authentication Tag (Poly1305 MAC)](#25-authentication-tag-poly1305-mac)
   - [2.6 Authenticated Encryption with Associated Data (AEAD)](#26-authenticated-encryption-with-associated-data-aead)
   - [2.7 Argon2id Memory-Hard Key Derivation](#27-argon2id-memory-hard-key-derivation)
3. [Key Hierarchy & Lifecycle Management](#3-key-hierarchy--lifecycle-management)
   - [3.1 Zero-Knowledge Envelope Architecture](#31-zero-knowledge-envelope-architecture)
   - [3.2 Separation of Verification and Vault Decryption](#32-separation-of-verification-and-vault-decryption)
   - [3.3 Zero-Downtime Master Password Rotation](#33-zero-downtime-master-password-rotation)
4. [AEAD Implementation: XChaCha20-Poly1305](#4-aead-implementation-xchacha20-poly1305)
   - [4.1 Why 192-bit Extended Nonces Matter](#41-why-192-bit-extended-nonces-matter)
   - [4.2 Detached vs. Combined Payloads](#42-detached-vs-combined-payloads)
   - [4.3 Associated Authenticated Data (AAD) Binding](#43-associated-authenticated-data-aad-binding)
5. [Secure Memory Architecture & Platform Isolation](#5-secure-memory-architecture--platform-isolation)
   - [5.1 `SecureBuffer` and `sodium_malloc`](#51-securebuffer-and-sodium_malloc)
   - [5.2 Memory Locking (`sodium_mlock`) & Page Swapping](#52-memory-locking-sodium_mlock--page-swapping)
   - [5.3 Memory Guard Pages & Canary Tripwires](#53-memory-guard-pages--canary-tripwires)
   - [5.4 Guaranteed Zeroization (`sodium_memzero`)](#54-guaranteed-zeroization-sodium_memzero)
   - [5.5 Platform Boundaries & Windows Memory Limits](#55-platform-boundaries--windows-memory-limits)
6. [Comprehensive Test Suite & Verification Matrix](#6-comprehensive-test-suite--verification-matrix)
   - [6.1 Positive Test Cases](#61-positive-test-cases)
   - [6.2 Negative Error Handling Cases](#62-negative-error-handling-cases)
   - [6.3 Tampering & Active Bit-Flip Attacks](#63-tampering--active-bit-flip-attacks)
   - [6.4 Key Rotation Verification](#64-key-rotation-verification)
7. [Production Readiness & Remaining Security Review Requirements](#7-production-readiness--remaining-security-review-requirements)

---

## 1. Cryptographic Philosophy & Core Tenets

The cryptographic subsystem of BlockVault adheres to strict defense-in-depth principles:

1. **Rule 1: Never Invent or Tweak Crypto.** BlockVault uses standard, peer-reviewed primitives implemented exclusively through **libsodium 1.0.22**, a widely audited C library.
2. **Rule 2: Zero Plaintext Exposure at Rest.** Plaintext passwords and secret tokens never hit the SQLite database or disk storage. Only authenticated binary ciphertext (`BLOB`) and nonces are persisted.
3. **Rule 3: Authenticate Everything (AEAD).** Unauthenticated encryption is catastrophic. BlockVault enforces **XChaCha20-Poly1305** authenticated encryption; tampering with a single bit in the ciphertext or metadata causes instant, safe decryption rejection.
4. **Rule 4: Zero Master Password Storage.** Neither the user's master password nor its raw hash is stored. Master password verification and vault key unwrapping are isolated via envelope encryption.
5. **Rule 5: Memory Hygiene & Anti-Dumping.** Sensitive keys in RAM are allocated with guard pages, locked against paging to swapfile, and cleared via compiler-barrier zeroization.

---

## 2. Beginner-Friendly Cryptography Primer

To understand how BlockVault protects secrets, here is a conceptual breakdown of each cryptographic building block:

```
+-----------------------------------------------------------------------------------------+
|                                    CRYPTOGRAPHIC WORKFLOW                               |
|                                                                                         |
|  Master Password + Random Salt (16B)                                                    |
|           │                                                                             |
|           ▼                                                                             |
|  [Argon2id Memory-Hard KDF]                                                             |
|           │                                                                             |
|           ├───► Hash Verification: H(KEK) ───► Database Verifier                        |
|           │                                                                             |
|           └───► Key Encryption Key (KEK, 32B)                                           |
|                       │                                                                 |
|                       ▼                                                                 |
|         Unwrap Encrypted Envelope [XChaCha20-Poly1305]                                  |
|                       │                                                                 |
|                       ▼                                                                 |
|           Data Encryption Key (DEK, 32B) ── (In-Memory Enclave)                        |
|                       │                                                                 |
|                       ▼                                                                 |
|         Encrypt/Decrypt Credential Passwords [XChaCha20-Poly1305]                       |
|           + Fresh Nonce (24B)                                                           |
|           + Associated Data (Record UUID)                                               |
|                       │                                                                 |
|                       ▼                                                                 |
|           [Ciphertext + 16B Poly1305 Tag] ──► SQLite BLOBs                              |
+-----------------------------------------------------------------------------------------+
```

### 2.1 Plaintext vs. Ciphertext
* **Plaintext:** The original, readable information (e.g. `MyBankPassword#2026!`). Anyone with read access to the file or RAM can see it.
* **Ciphertext:** The scrambled output of an encryption algorithm. To anyone without the secret key, it appears as indistinguishable pseudo-random noise.

### 2.2 Cryptographic Key
* A secret numerical value used by an encryption algorithm to transform plaintext into ciphertext and vice versa.
* BlockVault uses **256-bit keys** (32 bytes). A 256-bit key has $2^{256} \approx 1.15 \times 10^{77}$ possible combinations—more than the number of atoms in the observable universe. It is physically impossible to brute-force a 256-bit key using modern or near-future computing clusters.

### 2.3 Cryptographic Salt
* **What is it?** A sequence of 16 cryptographically random bytes generated anew for each user upon account creation.
* **Why is it needed?** Humans often choose common passwords. If two users choose the same password `Password123!` and you hash it directly, both hashes will be identical. An attacker can compute a **Rainbow Table** (precomputed table of billions of common password hashes) and crack all accounts at once.
* **How it helps:** By prepending a unique random salt to the password before hashing or key derivation ($P_{\text{salted}} = \text{Password} \parallel \text{Salt}$), two users with the exact same password will have completely different derived keys and verifiers. The attacker must recalculate the expensive derivation independently for every single account.

### 2.4 Cryptographic Nonce
* **What is it?** A **N**umber used **ONCE**. A 24-byte (192-bit) unique value generated before encrypting each individual field.
* **Why is it critical?** Stream ciphers like ChaCha20 generate a pseudo-random keystream from the Key and Nonce: $\text{Keystream} = \text{Cipher}(\text{Key}, \text{Nonce})$. The ciphertext is computed as $\text{Ciphertext} = \text{Plaintext} \oplus \text{Keystream}$.
* **The Catastrophic Replay Flaw:** If you encrypt two different messages $M_1$ and $M_2$ with the **same Key and same Nonce**, the keystream is identical:
  $$C_1 \oplus C_2 = (M_1 \oplus K) \oplus (M_2 \oplus K) = M_1 \oplus M_2$$
  The keystream cancels out completely! An attacker XORs the two ciphertexts together and can recover both original plaintexts without knowing the key!
* **The BlockVault Invariant:** A fresh, secure random nonce is generated for **every single encryption operation**. Nonce reuse is strictly prohibited.

### 2.5 Authentication Tag (Poly1305 MAC)
* **What is it?** A 16-byte (128-bit) cryptographic checksum generated by the **Poly1305** algorithm over the ciphertext and nonce.
* **Why is standard encryption dangerous without it?** Stream ciphers are malleable. If an attacker intercepts an encrypted wire transfer `TRANSFER $00100` and flips specific bits in the ciphertext, the decrypted plaintext will turn into `TRANSFER $99100`! Standard decryption would succeed without error.
* **How Poly1305 protects you:** The sender computes a Message Authentication Code (MAC) tag using a secret one-time key derived from the master key. Upon decryption, the recipient recomputes the tag. If even a **single bit** of the ciphertext or nonce has been altered, the tags will not match, and decryption aborts immediately without releasing any decrypted data.

### 2.6 Authenticated Encryption with Associated Data (AEAD)
* **AEAD** unites encryption (confidentiality) and authentication (integrity) into a single atomic cryptographic operation.
* **Associated Data (AAD):** Additional metadata that is **not encrypted** (remains public), but is **cryptographically authenticated** by the Poly1305 tag.
* In BlockVault, when an encrypted credential is saved, its `credential_id` UUID is passed as Associated Data. If an attacker copies an encrypted password from Record A and pastes it into Record B in the SQLite database, decryption will fail because the `credential_id` AAD does not match!

### 2.7 Argon2id Memory-Hard Key Derivation
* **The Problem with Traditional Hashing (SHA-256, MD5):** General-purpose hash functions are designed to be fast (tens of millions of hashes per second per GPU). An attacker with an RTX 4090 GPU rig can try $10^{10}$ passwords every second.
* **The Solution:** Argon2 won the Password Hashing Competition (PHC). It is deliberately designed to be **memory-hard**. It allocates tens or hundreds of megabytes of RAM and performs complex multi-pass permutations across memory banks.
* **Argon2id Hybrid:** Combines Argon2d (data-dependent access to resist GPU/ASIC cracking) and Argon2i (data-independent access to resist side-channel cache-timing attacks).
* **Profiles in BlockVault:**
  - `INTERACTIVE` (Default): ~64 MB RAM, 2 iteration passes. Balanced for mobile/desktop login responsiveness (~0.1s).
  - `MODERATE`: ~256 MB RAM, 3 iteration passes. High-security local vaults (~0.4s).
  - `SENSITIVE`: ~1024 MB RAM, 4 passes. Extreme-security sovereign air-gapped vaults.
  - `FAST_TEST`: 8 MB RAM, 1 pass. Designed for rapid automated CI/CD test execution while maintaining real Argon2id algorithmic execution.

---

## 3. Key Hierarchy & Lifecycle Management

BlockVault uses a multi-tier **Envelope Encryption Hierarchy** designed to achieve zero-knowledge security and painless key rotation:

```
[User Master Password]
         │
         ▼ (Argon2id + 16-byte random Salt)
[Key Encryption Key (KEK, 256-bit)]
         ├───► SHA-256(KEK) ───► Authentication Verifier in DB
         │
         ▼ (XChaCha20-Poly1305 Unwrapping)
[Encrypted Envelope in DB] ───► [Vault Data Encryption Key (DEK, 256-bit)]
                                          │
                                          ▼ (XChaCha20-Poly1305)
                        [Encrypted Credentials in Database]
```

### 3.1 Zero-Knowledge Envelope Architecture
1. **User Registration:**
   - The user inputs their master password.
   - A random 16-byte salt is generated via `CryptoManager::generateSalt()`.
   - Argon2id derives the 256-bit **Key Encryption Key (KEK)**.
   - A completely independent, random 256-bit **Data Encryption Key (DEK)** is generated via libsodium CSPRNG.
   - The DEK is encrypted under the KEK via XChaCha20-Poly1305 with Associated Data `"USER_DEK_ENVELOPE"`.
   - The user's verification token $\text{Verifier} = \text{SHA256}(\text{KEK}) \parallel \text{Envelope}(\text{DEK})$ is stored in the database.
   - The master password and raw KEK are immediately erased from memory.

### 3.2 Separation of Verification and Vault Decryption
* The master password itself is **never stored anywhere**, not even in RAM after derivation.
* Verification that the user typed the right master password is done by verifying that the derived candidate KEK produces the same SHA-256 digest stored in the database.
* Once verified, the candidate KEK unwraps the DEK envelope. The DEK is held in a protected `SecureBuffer` inside the in-memory session enclave for the duration of the user's unlocked session.

### 3.3 Zero-Downtime Master Password Rotation
In poorly architected password managers, if a user changes their master password, the system must read and re-encrypt every single credential in the database. If the app crashes halfway through, the database is corrupted and credentials are lost forever.

In BlockVault:
1. The user provides the old master password and new master password.
2. The old KEK unwraps the DEK into a locked memory buffer.
3. A fresh salt is generated; the new KEK is derived from the new master password.
4. The **DEK is re-wrapped under the new KEK** in a single atomic transaction.
5. **The DEK itself does not change.** All encrypted credentials in the database remain 100% valid and untouched!
6. Old keys are securely zeroized via `CryptoManager::clearSensitiveMemory()`.

---

## 4. AEAD Implementation: XChaCha20-Poly1305

### 4.1 Why 192-bit Extended Nonces Matter
Most modern systems use **AES-GCM** or standard **ChaCha20-Poly1305**, which utilize a **96-bit (12-byte) nonce**.
* With a 96-bit nonce, selecting random nonces introduces a collision risk due to the Birthday Paradox: after generating $2^{48}$ nonces (~280 trillion), the probability of collision is 50%. In distributed systems or long-lived vaults, accidental nonce reuse under random generation is a catastrophic failure.
* **XChaCha20** solves this by extending the nonce to **192 bits (24 bytes)** using the **HChaCha20** sub-key derivation block.
* With 192-bit nonces, the collision probability is negligible: you would need to generate $2^{96}$ random nonces before risking a collision. **Random nonces can be generated safely without maintaining centralized persistent counters.**

### 4.2 Detached vs. Combined Payloads
BlockVault implements two distinct serialization formats:

1. **Detached Payload (`EncryptedPayload`):**
   ```cpp
   struct EncryptedPayload {
       ByteVector nonce;      // 24 bytes (crypto_aead_xchacha20poly1305_ietf_NPUBBYTES)
       ByteVector ciphertext; // Exact length of plaintext
       ByteVector tag;        // 16 bytes (crypto_aead_xchacha20poly1305_ietf_ABYTES)
   };
   ```
   * *Usage:* Ideal for relational database schemas where nonces and tags can be indexed or stored in distinct columns or metadata envelopes.

2. **Combined Payload (`encryptCombined`):**
   ```
   [ 24-byte Nonce ][ N-byte Ciphertext ][ 16-byte Poly1305 Tag ]
   ```
   * *Usage:* Ideal for single-BLOB database storage, network wire transmission, or file exports.

### 4.3 Associated Authenticated Data (AAD) Binding
In BlockVault, whenever a credential field is encrypted, contextual metadata is bound into the Poly1305 calculation:
```cpp
EncryptedPayload enc = CryptoManager::encrypt_string(
    password, 
    vault_dek, 
    "CREDENTIAL_ID=" + cred_id + ";FIELD=PASSWORD"
);
```
If an adversary manipulates the database rows to swap fields or substitute an encrypted blob from a different account, decryption instantly fails with a `CryptoException`.

---

## 5. Secure Memory Architecture & Platform Isolation

Key management in RAM is as critical as encryption on disk. If a cryptographic key is stored in standard heap memory, it can be:
- Swapped to the Windows pagefile (`pagefile.sys`) or hibernation file (`hiberfil.sys`).
- Leaked into an unencrypted crash dump if the application crashes.
- Read after deallocation by other routines in the same process if memory is not wiped.

BlockVault implements the following memory safety architecture:

### 5.1 `SecureBuffer` and `sodium_malloc`
The [`SecureBuffer`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/crypto/SecureBuffer.hpp) RAII container wraps libsodium's specialized memory allocator:
- Allocates memory aligned on page boundaries.
- Places **guard pages** (unmapped virtual memory pages) immediately before and after the allocated block. Any buffer underflow or overflow triggers an instant operating system access violation (`SIGSEGV`), preventing memory corruption exploits.

### 5.2 Memory Locking (`sodium_mlock`) & Page Swapping
When `sodium_malloc()` is invoked, libsodium automatically issues `VirtualLock()` (on Windows) or `mlock()` (on POSIX). This instructs the OS kernel to pin the physical memory pages into RAM:
- The pages are **never written to the virtual memory swapfile/pagefile**.
- Secrets remain in volatile semiconductor RAM only.

### 5.3 Memory Guard Pages & Canary Tripwires
Canaries are stored before the allocated memory. Upon deallocation via `sodium_free()`, libsodium checks that the canaries were not overwritten. If a heap corruption occurred, the application aborts immediately.

### 5.4 Guaranteed Zeroization (`sodium_memzero`)
Standard `memset()` or `bzero()` calls are notoriously eliminated by modern optimizing compilers (dead-code elimination) when the memory is about to be freed.
`sodium_memzero()` uses architecture-specific assembly instructions and compiler barriers to guarantee that:
- Every byte of the key buffer is overwritten with `0x00`.
- The compiler cannot optimize the write away under `-O2` or `-O3`.

### 5.5 Platform Boundaries & Windows Memory Limits
* **Windows Working Set Limits:** On Windows, `VirtualLock()` has a per-process quota. Allocating gigabytes with `sodium_malloc` will fail once the quota is exhausted. `SecureBuffer` automatically detects this and falls back to standard heap memory while still guaranteeing `sodium_memzero()` upon destruction.
* **Hibernation Leakage (`hiberfil.sys`):** When Windows enters hibernation or hybrid sleep, it copies the entire physical RAM contents to `C:\hiberfil.sys`. While `VirtualLock` prevents pagefile swapping during active execution, it cannot prevent physical RAM hibernation dumps. To mitigate this in ultra-high-threat environments, Windows BitLocker full-disk encryption should always be enabled.

---

## 6. Comprehensive Test Suite & Verification Matrix

The test suite in [`tests/test_crypto.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/tests/test_crypto.cpp) executes an exhaustive set of positive, negative, and adversarial tampering tests.

### Test Execution Results
All test cases pass cleanly with zero failures and zero compiler warnings:

```
========================================================
 BLOCKVAULT C++20 LIBSODIUM CRYPTOGRAPHY TEST SUITE
========================================================

[CRYPTO TEST] test_initialization ... PASSED
[CRYPTO TEST] test_csprng_and_entropy ... PASSED
[CRYPTO TEST] test_argon2id_derivation ... PASSED
[CRYPTO TEST] test_aead_positive_roundtrips ... PASSED
[CRYPTO TEST] test_aead_negative_cases ... PASSED
[CRYPTO TEST] test_tampering_attacks ... PASSED
[CRYPTO TEST] test_secure_buffer_lifecycle ... PASSED
[CRYPTO TEST] test_key_rotation_envelope ... PASSED

--------------------------------------------------------
 CRYPTOGRAPHY RESULTS: 8 Passed, 0 Failed
========================================================
```

### 6.1 Positive Test Cases
1. **Libsodium Safe Initialization:** Verified that `CryptoManager::initialize()` initializes libsodium safely, is thread-safe across concurrent threads, and is idempotent.
2. **CSPRNG Uniqueness & Entropy:** Verified that consecutive salt and nonce generations are strictly unique across 100 test iterations with zero collisions.
3. **Argon2id Determinism & Avalanche:** Verified that same password and salt produce identical 32-byte keys, while a 1-character difference in password produces an orthogonal key with >50% bit flip distribution.
4. **AEAD Roundtrip Variety:** Verified end-to-end encryption/decryption roundtrips across:
   - Standard short credentials (16–64 bytes).
   - Empty plaintext payloads (zero bytes).
   - Large notes and secure attachments (64 KB).
   - Unicode, emojis, and non-ASCII multi-byte strings (UTF-8).
   - Combined single-blob payloads.

### 6.2 Negative Error Handling Cases
1. **Wrong Decryption Key:** Decrypting with an incorrect 32-byte key throws `CryptoException`.
2. **Wrong Decryption Nonce:** Decrypting with an incorrect 24-byte nonce throws `CryptoException`.
3. **Mismatched Associated Data (AAD):** Encrypting with `USER_SESSION=42` and attempting decryption with `USER_SESSION=99` or empty AAD throws `CryptoException`.
4. **Truncated Payloads:** Truncated ciphertext or tag throws `CryptoException`.

### 6.3 Tampering & Active Bit-Flip Attacks
1. **Payload Bit-Flip:** Flipping the least significant bit of the ciphertext payload causes Poly1305 authentication verification to fail; `CryptoException` is thrown and zero plaintext is released.
2. **Tag Bit-Flip:** Flipping the most significant bit of the 16-byte Poly1305 tag causes instant rejection.
3. **AAD Tampering:** Changing a single character in the authenticated metadata triggers rejection.

### 6.4 Key Rotation Verification
Simulated the master password change workflow: old KEK unwraps vault DEK, new KEK wraps DEK under a fresh salt, and old keys are zeroized. Existing credentials remain decryptable under the rotated master password without re-encrypting the database.

---

## 7. Production Readiness & Remaining Security Review Requirements

> [!WARNING]
> **Passing automated unit tests does NOT guarantee production-grade security.**  
> Cryptographic correctness is a necessary prerequisite, but complete system security depends on operational environments, side-channel resilience, memory dump hygiene, and formal independent auditing.

Before deploying BlockVault into an enterprise production environment, the following security-review requirements must be satisfied:

1. **Third-Party Cryptographic Audit:**
   - Formal source code audit conducted by certified cryptographic security firms (e.g. Trail of Bits, Cure53, NCC Group).
   - Review of timing invariance, memory boundaries, and state machines.
2. **Side-Channel & Microarchitectural Analysis:**
   - Verify that all password comparison paths, tag verifications, and key handling remain strictly constant-time under speculative execution (Spectre/Meltdown mitigations).
   - Inspect disassembly to ensure compiler vectorization did not introduce branch-based early exists in `constant_time_equals`.
3. **Hardware Security Module (HSM) / Secure Enclave Integration:**
   - Future roadmap should integrate Windows CNG (Cryptography Next Generation) with TPM 2.0 (Trusted Platform Module) or Apple Secure Enclave / Linux PKCS#11 to seal the KEK in dedicated tamper-resistant hardware chips.
4. **Crash Dump Sanitization:**
   - Configure Windows Error Reporting (WER) on client machines to suppress full memory dumps (`WerAddExcludedApplication` or disabling crash dumps for `blockvault.exe`) to prevent keys from persisting in `C:\Windows\Minidump` upon a segmentation fault.
5. **Post-Quantum Cryptography (PQC) Roadmap:**
   - Prepare migration pathways toward NIST Post-Quantum standards (ML-KEM / Kyber for asymmetric key exchange, ML-DSA / Dilithium for digital signatures) once libsodium standardizes PQC primitives.

---

## 8. Summary of Cryptographic Specifications

| Component | Primitive / Standard | Parameter / Key Size | Purpose |
| :--- | :--- | :--- | :--- |
| **Crypto Library** | libsodium 1.0.22 | Official MinGW Static Lib | Core engine |
| **Password KDF** | Argon2id v1.3 (`crypto_pwhash`) | 64 MB RAM, 2 Passes (Interactive) | Derives KEK from master password |
| **Symmetric Cipher** | XChaCha20 (`crypto_aead_xchacha20poly1305_ietf`) | 256-bit (32 bytes) | High-speed stream encryption |
| **Extended Nonce** | HChaCha20 Nonce Extension | 192-bit (24 bytes) | Eliminates random nonce collision risk |
| **Message Authentication** | Poly1305 One-Time MAC | 128-bit (16 bytes) | Authenticates ciphertext & AAD |
| **Hashing Engine** | SHA-256 (`crypto_hash_sha256`) | 256-bit (32 bytes) | Hash-linked audit ledger & verifier |
| **CSPRNG** | Libsodium `randombytes_buf` | OS System Entropy Source | Random salts, nonces, keys & tokens |
| **Memory Security** | `SecureBuffer` (`sodium_malloc`) | Guard pages, `VirtualLock` | Anti-dump, anti-swap, guaranteed zeroize |

