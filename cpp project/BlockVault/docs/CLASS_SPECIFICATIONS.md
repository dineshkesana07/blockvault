# BlockVault – C++20 Core Architecture & Class Specifications

> **Language Standard:** C++20 (ISO/IEC 14882:2020)  
> **Compiler:** GCC 14.2 / Clang 19.1 (MinGW-w64 UCRT)  
> **Persistence:** SQLite3 3.53+ (WAL Mode, RAII)  
> **Security Model:** Zero-Knowledge Envelope Encryption & SHA-256 Hash Chain  
> **Directory Structure:** `include/`, `src/`, `tests/`, `docs/`, `bin/`

---

## Table of Contents
1. [`User`](#1-class-user)
2. [`Credential`](#2-class-credential)
3. [`Block`](#3-class-block)
4. [`Vault`](#4-class-vault)
5. [`SecureBuffer`](#5-class-securebuffer)
6. [`CryptoManager`](#6-class-cryptomanager)
7. [`PasswordGenerator`](#7-class-passwordgenerator)
8. [`DatabaseManager`](#8-class-databasemanager)
9. [`AuditLedger`](#9-class-auditledger)
10. [`SessionManager`](#10-class-sessionmanager)
11. [`AuthManager`](#11-class-authmanager)
12. [`CredentialManager`](#12-class-credentialmanager)

---

## 1. Class: `User`
**Header:** [`include/models/User.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/models/User.hpp)  
**Implementation:** [`src/models/User.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/models/User.cpp)

### Responsibility
Represents an individual authenticated account within BlockVault. It encapsulates user identity, cryptographic salt, password verification material, and the encrypted Data Encryption Key (DEK) wrapped under the user's Key Encryption Key (KEK).

### Why the Class is Required
A core tenet of Zero-Knowledge security is that the server must never know the master password or hold an unencrypted DEK. `User` provides a strictly typed domain model storing only the mathematical verifier and ciphertext envelope, ensuring zero plaintext exposure even if the database is leaked.

### Data Members
- `m_id` (`std::string`): UUID v4 uniquely identifying the user.
- `m_email` (`std::string`): Unique email address used for login.
- `m_password_hash` (`std::string`): 64-character hex hash computed over the derived KEK ($H(\text{KEK})$) used to verify master password entry without storing the master password.
- `m_salt` (`ByteVector`): 16 cryptographically random bytes used as salt for Argon2id/PBKDF2 key derivation.
- `m_encrypted_dek` (`EncryptedPayload`): The 256-bit symmetric vault key (DEK) encrypted by the KEK with a 24-byte nonce and 16-byte authentication tag.
- `m_created_at` (`Timestamp`): High-resolution UTC timestamp when the user registered.
- `m_updated_at` (`Timestamp`): High-resolution UTC timestamp of the latest profile/password update.

### Member Functions
- `User()`: Default constructor; initializes timestamps to current UTC time.
- `User(id, email, hash, salt, encrypted_dek, created_at, updated_at)`: Parameterized constructor.
- `id() const`, `email() const`, `password_hash() const`, `salt() const`, `encrypted_dek() const`, `created_at() const`, `updated_at() const`: Const-correct getters.
- `set_email()`, `set_password_hash()`, `set_salt()`, `set_encrypted_dek()`: Mutators that update fields and automatically touch `m_updated_at`.
- `touch()`: Updates `m_updated_at` to the current system clock time.

### Small Usage Example
```cpp
#include "models/User.hpp"
#include "crypto/CryptoManager.hpp"

// Creating a new user model
blockvault::EncryptedPayload dummy_envelope;
blockvault::ByteVector salt = blockvault::CryptoManager::generate_random_bytes(16);

blockvault::User user(
    "usr_4f2b189a",
    "alice@blockvault.sec",
    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
    salt,
    dummy_envelope
);

std::cout << "User registered: " << user.email() << " (ID: " << user.id() << ")\n";
```

---

## 2. Class: `Credential`
**Header:** [`include/models/Credential.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/models/Credential.hpp)  
**Implementation:** [`src/models/Credential.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/models/Credential.cpp)

### Responsibility
Encapsulates an individual credential item saved inside the vault. It models metadata (service title, URL, username, category, favorite state) alongside authenticated ciphertext payloads for the password and sensitive notes.

### Why the Class is Required
Prevents plaintext credential exposure at rest. By encapsulating encrypted blobs rather than plain strings, the system guarantees that credentials remain cryptographically sealed until explicitly decrypted in memory during an active, unlocked session.

### Data Members
- `m_id` (`std::string`): UUID v4 identifier.
- `m_user_id` (`std::string`): Foreign key identifying the owning user (tenant isolation).
- `m_title` (`std::string`): Human-readable service label (e.g. "GitHub Enterprise").
- `m_website_url` (`std::string`): Destination login endpoint URL.
- `m_username` (`std::string`): Account username or login email.
- `m_encrypted_password` (`EncryptedPayload`): DEK-encrypted password with 24-byte nonce and 16-byte Poly1305 MAC tag.
- `m_encrypted_notes` (`EncryptedPayload`): DEK-encrypted secure notes.
- `m_category` (`CredentialCategory`): Classification enum (`Login`, `SecureNote`, `CreditCard`, `Server`, `Identity`).
- `m_favorite` (`bool`): Boolean flag indicating pinned status.
- `m_created_at` (`Timestamp`): Record creation UTC time.
- `m_updated_at` (`Timestamp`): Last modification UTC time.

### Member Functions
- `Credential()`: Default constructor; initializes timestamps and default `Login` category.
- `Credential(...)`: Parameterized constructor taking all fields.
- `id() const`, `user_id() const`, `title() const`, `website_url() const`, `username() const`, `encrypted_password() const`, `encrypted_notes() const`, `category() const`, `is_favorite() const`: Observers.
- `set_title()`, `set_website_url()`, `set_username()`, `set_encrypted_password()`, `set_encrypted_notes()`, `set_category()`, `set_favorite()`: Mutators updating timestamps.

### Small Usage Example
```cpp
#include "models/Credential.hpp"

blockvault::Credential cred(
    "cred_99a8b1",
    "usr_4f2b189a",
    "AWS Production",
    "https://aws.amazon.com",
    "sysadmin@corp.internal",
    encrypted_password_blob,
    encrypted_notes_blob,
    blockvault::CredentialCategory::Server,
    true
);

std::cout << "Credential: " << cred.title() << " for user " << cred.user_id() << "\n";
```

---

## 3. Class: `Block`
**Header:** [`include/models/Block.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/models/Block.hpp)  
**Implementation:** [`src/models/Block.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/models/Block.cpp)

### Responsibility
Models a single immutable node in the append-only, SHA-256 hash-chained local audit ledger.

### Why the Class is Required
Provides mathematical tamper-evidence. Each block cryptographically binds its own transaction data to the hash of the preceding block:
$$\text{CurrentHash} = \text{SHA256}(\text{Index} \parallel \text{Timestamp} \parallel \text{EventType} \parallel \text{UserID} \parallel \text{Details} \parallel \text{PrevHash})$$
Any retrospective alteration, record deletion, or row injection permanently breaks the chain.

### Data Members
- `m_index` (`uint64_t`): 0-based sequential counter ($0 = \text{Genesis}$).
- `m_timestamp` (`Timestamp`): High-resolution UTC time when the event occurred.
- `m_event_type` (`AuditEventType`): Categorized security event enum.
- `m_user_id` (`std::string`): Actor account UUID.
- `m_entity_id` (`std::string`): Target credential or session ID.
- `m_details` (`std::string`): Human-readable event description (sanitized; never containing secrets).
- `m_previous_hash` (`std::string`): 64-character hex hash of block $n-1$.
- `m_current_hash` (`std::string`): 64-character hex hash calculated over this block.

### Member Functions
- `canonical_payload() const`: Formats all block fields into a deterministic colon-delimited string.
- `compute_hash() const`: Computes the SHA-256 digest of the canonical payload.
- `is_valid() const`: Performs constant-time verification that `m_current_hash == compute_hash()`.
- `create_genesis_block()`: Static factory constructing the Genesis block ($Index = 0$, $PrevHash = 64\times\text{'0'}\text{s}$).

### Small Usage Example
```cpp
#include "models/Block.hpp"

blockvault::Block b1(
    1,
    blockvault::AuditEventType::CredentialCreated,
    "usr_4f2b189a",
    "cred_99a8b1",
    "Created credential for GitHub Enterprise",
    genesis_block.current_hash()
);

assert(b1.is_valid());
std::cout << "Block #1 Hash: " << b1.current_hash() << "\n";
```

---

## 4. Class: `Vault`
**Header:** [`include/models/Vault.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/models/Vault.hpp)  
**Implementation:** [`src/models/Vault.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/models/Vault.cpp)

### Responsibility
In-memory domain aggregate and collection container representing a user's complete credential inventory.

### Why the Class is Required
Provides fast querying, category grouping, search filtering, and favorites lookup without making repeated disk queries.

### Data Members
- `m_user_id` (`std::string`): Owning user identifier.
- `m_credentials` (`std::unordered_map<std::string, Credential>`): Hash table of credentials indexed by credential ID for $O(1)$ lookups.

### Member Functions
- `add_credential(const Credential&)`: Adds a credential; throws `VaultException` if duplicate ID.
- `update_credential(const Credential&)`: Updates existing credential; throws if not found.
- `remove_credential(id)`: Removes item; returns `true` if deleted.
- `get_credential(id) const`: Returns `std::optional<Credential>`.
- `all() const`: Returns `std::vector<Credential>` of all items.
- `filter_by_category(category) const`: Returns items matching specified category.
- `search(query) const`: Case-insensitive search across title, username, and website URL.
- `favorites() const`: Returns items flagged as favorites.

### Small Usage Example
```cpp
#include "models/Vault.hpp"

blockvault::Vault vault("usr_4f2b189a");
vault.add_credential(cred1);
vault.add_credential(cred2);

auto search_results = vault.search("github");
std::cout << "Found " << search_results.size() << " matching items.\n";
```

---

## 5. Class: `SecureBuffer`
**Header:** [`include/crypto/SecureBuffer.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/crypto/SecureBuffer.hpp)  
**Implementation:** [`src/crypto/SecureBuffer.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/crypto/SecureBuffer.cpp)

### Responsibility
RAII-governed memory container specifically designed for volatile cryptographic key material (KEK, DEK, master passphrases).

### Why the Class is Required
Standard heap vectors (`std::vector<uint8_t>`) and strings do not guarantee memory zeroization upon destruction—compilers routinely optimize away trailing `memset` calls via dead-code elimination. `SecureBuffer` uses volatile memory loops and prevents accidental secret duplication by deleting copy constructors.

### Data Members
- `m_data` (`uint8_t*`): Pointer to dynamically allocated memory.
- `m_size` (`size_t`): Buffer size in bytes.

### Member Functions
- `SecureBuffer(size_t)`: Allocates zeroed buffer.
- `SecureBuffer(const uint8_t*, size_t)` / `SecureBuffer(const ByteVector&)`: Ingests data.
- `~SecureBuffer()`: Calls `zeroize()` and frees heap memory.
- `operator=(SecureBuffer&&)`: Move semantics; transfers buffer ownership and zeroes donor.
- `data()`, `size()`, `empty()`: Observers.
- `zeroize()`: Wipes memory using volatile pointer semantics immune to compiler elimination.

### Small Usage Example
```cpp
#include "crypto/SecureBuffer.hpp"

{
    blockvault::SecureBuffer key(32);
    // Fill key with secret cryptographic data...
} // Destructor automatically zeroizes key from RAM here!
```

---

## 6. Class: `CryptoManager`
**Header:** [`include/crypto/CryptoManager.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/crypto/CryptoManager.hpp)  
**Implementation:** [`src/crypto/CryptoManager.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/crypto/CryptoManager.cpp)

### Responsibility
Centralized cryptographic engine providing CSPRNG random generation, FIPS 180-4 compliant SHA-256 hashing, RFC 2898 PBKDF2/Argon2id key derivation, Authenticated Encryption (AEAD), and constant-time memory comparisons.

### Why the Class is Required
Isolates all low-level cryptographic mathematics into a single audited service, eliminating error-prone ad-hoc encryption across application logic.

### Data Members
Pure stateless service class. Contains no mutable static state.

### Member Functions
- `generate_random_bytes(size)`: Cryptographically secure random bytes via OS entropy (`CryptGenRandom` / `std::random_device`).
- `generate_random_hex(size)`: Returns random hex string.
- `generate_uuid_v4()`: Returns RFC 4122 compliant UUID v4 string.
- `sha256_hex(input)`: Computes 64-character hex SHA-256 digest.
- `derive_key(password, salt, key_len, iterations)`: Derives symmetric key using memory-hard key derivation into a `SecureBuffer`.
- `encrypt(plaintext, key, aad)`: Authenticated encryption with 24-byte nonce and 16-byte MAC tag.
- `decrypt(payload, key, aad)`: Verifies MAC tag *first*; decrypts ciphertext if authentic; throws `CryptoException` on bit-flipping.
- `constant_time_equals(a, b)`: Execution time invariant to character mismatch location, neutralizing timing side-channels.
- `to_hex()` / `from_hex()`: Hex encoding and decoding helpers.

### Small Usage Example
```cpp
#include "crypto/CryptoManager.hpp"

blockvault::SecureBuffer key = blockvault::CryptoManager::derive_key("MyPassword123!", salt);
blockvault::EncryptedPayload enc = blockvault::CryptoManager::encrypt_string("SecretText", key, "AAD");
std::string dec = blockvault::CryptoManager::decrypt_string(enc, key, "AAD");
assert(dec == "SecretText");
```

---

## 7. Class: `PasswordGenerator`
**Header:** [`include/crypto/PasswordGenerator.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/crypto/PasswordGenerator.hpp)  
**Implementation:** [`src/crypto/PasswordGenerator.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/crypto/PasswordGenerator.cpp)

### Responsibility
Generates cryptographically random passwords, computes Shannon entropy, and generates 24-word emergency recovery mnemonics.

### Why the Class is Required
Provides unbiased random character selection (rejection sampling eliminating modulo bias), ensures diverse character pools, and evaluates password strength metrics objectively.

### Data Members
Stateless generator class operating on `PasswordOptions` structs.

### Member Functions
- `generate(const PasswordOptions&)`: Generates password adhering to length, character classes, and ambiguous character rules. Employs cryptographic Fisher-Yates shuffle.
- `assess(password)`: Returns `PasswordAssessment` containing Shannon entropy bits, strength rating, and recommendations.
- `calculate_entropy(password)`: Calculates exact Shannon entropy:
  $$H = L \times \log_2(\text{Pool Size})$$
- `generate_recovery_phrase()`: Generates a 24-word BIP-39 mnemonic recovery phrase.

### Small Usage Example
```cpp
#include "crypto/PasswordGenerator.hpp"

blockvault::PasswordOptions opts;
opts.length = 24;
opts.include_symbols = true;

std::string pwd = blockvault::PasswordGenerator::generate(opts);
auto assessment = blockvault::PasswordGenerator::assess(pwd);
std::cout << "Generated: " << pwd << " (Entropy: " << assessment.entropy_bits << " bits)\n";
```

---

## 8. Class: `DatabaseManager`
**Header:** [`include/db/DatabaseManager.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/db/DatabaseManager.hpp)  
**Implementation:** [`src/db/DatabaseManager.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/db/DatabaseManager.cpp)

### Responsibility
Manages the SQLite database connection, WAL pragmas, schema migrations, and prepared SQL statement execution with typed parameter binding.

### Why the Class is Required
Centralizes persistent storage, enforces referential integrity, guarantees zero SQL injection, and prevents SQLite concurrency deadlocks via Write-Ahead Logging (WAL).

### Data Members
- `m_db` (`sqlite3*`): Raw SQLite connection handle.
- `m_db_path` (`std::string`): File path or `:memory:`.
- `m_is_open` (`bool`): Connection state flag.

### Member Functions
- `open(db_path)` / `close()`: Connection lifecycle with WAL configuration.
- `insert_user()`, `get_user_by_id()`, `get_user_by_email()`, `update_user()`, `delete_user()`: User CRUD.
- `insert_credential()`, `get_credential_by_id()`, `get_credentials_for_user()`, `update_credential()`, `delete_credential()`: Credential CRUD with blob serialization.
- `insert_block()`, `get_last_block()`, `get_all_blocks()`, `get_block_count()`: Audit block queries.
- `insert_session()`, `validate_session()`, `delete_session()`, `purge_expired_sessions()`: Session storage.
- `begin_transaction()`, `commit_transaction()`, `rollback_transaction()`: Atomic transactions.

### Small Usage Example
```cpp
#include "db/DatabaseManager.hpp"

blockvault::DatabaseManager db("vault.db");
auto user_opt = db.get_user_by_email("user@blockvault.sec");
if (user_opt.has_value()) {
    std::cout << "Loaded user UUID: " << user_opt->id() << "\n";
}
```

---

## 9. Class: `AuditLedger`
**Header:** [`include/ledger/AuditLedger.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/ledger/AuditLedger.hpp)  
**Implementation:** [`src/ledger/AuditLedger.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/ledger/AuditLedger.cpp)

### Responsibility
Manages the append-only SHA-256 hash-chained audit journal. It writes security transaction blocks and executes verification scans across the ledger.

### Why the Class is Required
Provides audit compliance and non-repudiation. Guarantees that unauthorized changes to historical security records can be mathematically detected.

### Data Members
- `m_db` (`std::shared_ptr<DatabaseManager>`): Shared reference to the database engine.

### Member Functions
- `AuditLedger(shared_ptr<DatabaseManager>)`: Constructor ensuring Genesis block initialization.
- `record_event(userId, eventType, entityId, details)`: Appends a new block linked to the latest current hash.
- `verify_integrity()`: Sequentially traverses all blocks, recalculating hashes to verify link continuity and data integrity.
- `get_all_records()`, `get_user_records()`, `get_latest_block()`: History query methods.

### Small Usage Example
```cpp
#include "ledger/AuditLedger.hpp"

auto ledger = std::make_shared<blockvault::AuditLedger>(db);
ledger->record_event("usr_123", blockvault::AuditEventType::VaultUnlocked, "usr_123", "Session unlock");

auto result = ledger->verify_integrity();
std::cout << "Ledger valid: " << std::boolalpha << result.is_valid << "\n";
```

---

## 10. Class: `SessionManager`
**Header:** [`include/auth/SessionManager.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/auth/SessionManager.hpp)  
**Implementation:** [`src/auth/SessionManager.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/auth/SessionManager.cpp)

### Responsibility
Governs active user sessions and in-memory cryptographic enclaves. Holds the decrypted DEK in volatile memory during an unlocked session and enforces automatic inactivity timeouts.

### Why the Class is Required
Allows users to perform multiple vault operations without re-entering their master password for every action, while ensuring that the decrypted DEK is securely wiped if the user leaves their computer idle.

### Data Members
- `m_db` (`std::shared_ptr<DatabaseManager>`): Database connection.
- `m_active_sessions` (`std::unordered_map<std::string, ActiveSession>`): In-memory active session table holding tokens, user IDs, expiration timestamps, and decrypted `active_dek` buffers.
- `m_inactivity_timeout_seconds` (`int64_t`): Inactivity duration (default: 900s / 15 minutes).

### Member Functions
- `create_session(user_id, client_ip, duration)`: Generates 256-bit random session token and initializes session.
- `validate_session(token)`: Validates token and checks inactivity/hard expiration.
- `unlock_vault(token, dek)`: Stores decrypted DEK into volatile session enclave.
- `lock_vault(token)`: Wipes and zeroes DEK from RAM.
- `is_vault_unlocked(token)`: Returns whether vault is unlocked and not timed out.
- `get_dek(token)`: Retrieves reference to in-memory `SecureBuffer` DEK; throws if locked.
- `terminate_session(token)`: Terminates and wipes session.

### Small Usage Example
```cpp
#include "auth/SessionManager.hpp"

blockvault::SessionManager session_mgr(db, 900);
std::string token = session_mgr.create_session("usr_123", "127.0.0.1");
session_mgr.unlock_vault(token, std::move(unwrapped_dek));

if (session_mgr.is_vault_unlocked(token)) {
    const auto& dek = session_mgr.get_dek(token);
    // Use DEK to decrypt credentials...
}
```

---

## 11. Class: `AuthManager`
**Header:** [`include/auth/AuthManager.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/auth/AuthManager.hpp)  
**Implementation:** [`src/auth/AuthManager.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/auth/AuthManager.cpp)

### Responsibility
High-level service coordinating zero-knowledge authentication, master password verification, envelope key unwrapping, session establishment, and audit event emission.

### Why the Class is Required
Decouples authentication business logic from transport protocols (HTTP/CLI) and lower-level database routines, providing a single cohesive point of entry for user identity management.

### Data Members
- `m_db` (`std::shared_ptr<DatabaseManager>`): Database engine.
- `m_session_mgr` (`std::shared_ptr<SessionManager>`): Session enclave manager.
- `m_ledger` (`std::shared_ptr<AuditLedger>`): Audit trail service.

### Member Functions
- `register_user(email, masterPassword)`: Derives KEK, wraps generated DEK, persists user, logs `UserRegistered`.
- `login(email, masterPassword, clientIp)`: Verifies master password hash in constant time, unwraps DEK into `SessionManager`, logs `UserLoginSuccess`, returns session token.
- `unlock_vault(token, masterPassword)`: Manually unlocks an existing locked session using master password.
- `lock_vault(token)`: Manually locks session vault.
- `logout(token)`: Terminates session and cleans memory.
- `get_authenticated_user(token)`: Retrieves user profile for validated token.

### Small Usage Example
```cpp
#include "auth/AuthManager.hpp"

blockvault::AuthManager auth(db, session_mgr, ledger);
std::string token = auth.login("alice@blockvault.sec", "SuperSecretPass123!", "192.168.1.1");
std::cout << "Logged in! Bearer token: " << token << "\n";
```

---

## 12. Class: `CredentialManager`
**Header:** [`include/vault/CredentialManager.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/vault/CredentialManager.hpp)  
**Implementation:** [`src/vault/CredentialManager.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/vault/CredentialManager.cpp)

### Responsibility
Orchestrates authenticated CRUD operations on credentials: authenticating user sessions, retrieving active DEKs, encrypting fields before database writes, decrypting fields on read, enforcing tenant boundaries, and logging audit events.

### Why the Class is Required
Enforces tenant isolation (`WHERE id = ? AND user_id = ?`) and guarantees that no credential is ever written unencrypted to the database or accessed without an unlocked cryptographic enclave.

### Data Members
- `m_db` (`std::shared_ptr<DatabaseManager>`)
- `m_session_mgr` (`std::shared_ptr<SessionManager>`)
- `m_ledger` (`std::shared_ptr<AuditLedger>`)

### Member Functions
- `create_credential(token, title, url, username, password, notes, category, favorite)`: Encrypts password and notes using DEK; persists record; logs `CredentialCreated`.
- `get_decrypted_credential(token, credential_id)`: Verifies ownership; decrypts password and notes; logs `CredentialRead`; returns `DecryptedCredential`.
- `get_vault(token)`: Returns a populated `Vault` domain container.
- `update_credential(...)`: Re-encrypts payload and updates database; logs `CredentialUpdated`.
- `delete_credential(token, credential_id)`: Removes item; logs `CredentialDeleted`.
- `toggle_favorite(token, credential_id)`: Toggles pinned status.

### Small Usage Example
```cpp
#include "vault/CredentialManager.hpp"

blockvault::CredentialManager cred_mgr(db, session_mgr, ledger);

// Create encrypted credential
auto cred = cred_mgr.create_credential(
    token, "GitHub", "https://github.com", "dineshbabu",
    "ghp_P@ssword99!", "Personal developer token"
);

// Decrypt on demand
auto dec = cred_mgr.get_decrypted_credential(token, cred.id());
std::cout << "Decrypted password: " << dec.password << "\n";
```

---
*End of Class Specifications — BlockVault Core Architecture Engineering Team.*

