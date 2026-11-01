# BlockVault – Hash-Linked Audit Ledger Specification

> **Document Classification:** Tamper-Evident Ledger Engineering Specification  
> **Target Engine:** SQLite 3.53+ Persistent Journal & Libsodium SHA-256 (`crypto_hash_sha256`)  
> **Language Standard:** C++20 (ISO/IEC 14882:2020)  
> **Architecture Principle:** Local Tamper-Evident Hash Chain (NOT a Decentralized Blockchain)  
> **Source Modules:** [`include/ledger/AuditLedger.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/ledger/AuditLedger.hpp), [`src/ledger/AuditLedger.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/ledger/AuditLedger.cpp), [`include/models/Block.hpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/include/models/Block.hpp), [`src/models/Block.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/models/Block.cpp), [`tests/test_audit_ledger.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/tests/test_audit_ledger.cpp)

---

## Table of Contents
1. [Architectural Overview & Core Clarification](#1-architectural-overview--core-clarification)
2. [The Genesis Record](#2-the-genesis-record)
3. [Mathematical Hash Chaining & Canonical Serialization](#3-mathematical-hash-chaining--canonical-serialization)
4. [Audit Event Catalog & Zero-Leakage Policy](#4-audit-event-catalog--zero-leakage-policy)
5. [Cryptographic Verification Engine & Tamper Detection](#5-cryptographic-verification-engine--tamper-detection)
6. [Automated Test Suite & Tamper Scenarios](#6-automated-test-suite--tamper-scenarios)
7. [Limitations of Local Hash Chains & Independent Anchoring](#7-limitations-of-local-hash-chains--independent-anchoring)
   - [7.1 The Threat of Local History Rewriting](#71-the-threat-of-local-history-rewriting)
   - [7.2 Truncation & State Rollback Attacks](#72-truncation--state-rollback-attacks)
   - [7.3 Solutions: External Notarization & Anchoring Protocols](#73-solutions-external-notarization--anchoring-protocols)

---

## 1. Architectural Overview & Core Clarification

### Crucial Engineering Clarification:
The BlockVault Audit Ledger is an **append-only, cryptographic hash-linked journal**. 
* It is **NOT** a decentralized cryptocurrency blockchain.
* It does **NOT** use Proof-of-Work (PoW), Proof-of-Stake (PoS), distributed P2P gossiping, smart contracts, or native tokens.
* Using the term "blockchain" to describe a local hash chain is technically misleading. BlockVault's ledger is a **tamper-evident audit trail** inspired by Haber and Stornetta's seminal 1991 paper (*"How to Time-Stamp a Digital Document"*), implemented directly on top of embedded SQLite.

```
+─────────────────────────────────────────────────────────────────────────────────────────────+
|                                  HASH-LINKED AUDIT JOURNAL                                  |
|                                                                                             |
|   ┌───────────────────────────┐     ┌───────────────────────────┐     ┌───────────────────┐ |
|   │         Block #0          │     │         Block #1          │     │      Block #2     │ |
|   │       GENESIS BLOCK       │     │       VAULT_CREATED       │     │  CREDENTIAL_ADDED │ |
|   │                           │     │                           │     │                   │ |
|   │ Index: 0                  │     │ Index: 1                  │     │ Index: 2          │ |
|   │ Time:  1700000000         │     │ Time:  1700000005         │     │ Time:  1700000020 │ |
|   │ Prev:  000000000000...000 │     │ Prev:  a7b39f... [Hash#0] │     │ Prev: 4f82c1...   │ |
|   │ Hash:  a7b39f...          ├────►│ Hash:  4f82c1...          ├────►│ Hash: 9e10d4...   │ |
|   └───────────────────────────┘     └───────────────────────────┘     └───────────────────┘ |
+─────────────────────────────────────────────────────────────────────────────────────────────+
```

### Core Security Invariants:
1. **Monotonically Increasing Sequence:** Records are strictly numbered ($0, 1, 2, \dots$). Any missing number signals deletion.
2. **Deterministic Cryptographic Linkage:** Each record incorporates the exact 64-character hexadecimal SHA-256 digest of the immediately preceding record.
3. **Zero-Secret Logging:** Plaintext passwords, master keys, encryption keys, or sensitive credential values are **strictly forbidden** from appearing in audit metadata.
4. **Time Invariance:** Timestamp sequence is verified to ensure chronological monotonicity (no retroactive backdating).

---

## 2. The Genesis Record

Every cryptographic hash chain must originate from an unambiguous, hardcoded initial state called the **Genesis Block** (Index 0).

In BlockVault, the Genesis block is constructed deterministically via [`Block::create_genesis_block()`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/models/Block.cpp#L73-L82):

```cpp
Block Block::create_genesis_block() {
    return Block(
        0,                                                                  // Index 0
        AuditEventType::Genesis,                                            // Event Type
        "SYSTEM",                                                           // Actor User ID
        "00000000-0000-0000-0000-000000000000",                             // Entity UUID
        "BlockVault Local Cryptographic Audit Ledger Initialized",           // Metadata
        "0000000000000000000000000000000000000000000000000000000000000000" // 64 zero nibbles
    );
}
```

### Why a Fixed Genesis Block is Required:
Without a predefined Genesis anchor, an attacker could wipe the entire database and create an entirely new fake ledger starting at whatever block they desire. The Genesis record anchors the chain to an immutable, known root state.

---

## 3. Mathematical Hash Chaining & Canonical Serialization

### 3.1 Canonical Serialization Format
Hashing arbitrary struct memory directly with `reinterpret_cast` or `sizeof` is dangerous in C++ because compiler struct padding, endianness, and memory alignment can vary across architectures.

BlockVault uses **deterministic canonical string serialization** in [`Block::canonical_payload()`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/models/Block.cpp#L49-L63):

$$\text{CanonicalPayload} = \text{Index} \parallel \text{EpochSeconds} \parallel \text{EventType} \parallel \text{UserID} \parallel \text{EntityID} \parallel \text{Details} \parallel \text{PreviousHash}$$

Format:
```
<Index>:<EpochSeconds>:<EventTypeString>:<UserID>:<EntityID>:<Details>:<PreviousHash>
```

Example Canonical Payload for Block #1:
```
1:1730498400:VAULT_CREATED:550e8400-e29b-41d4-a716-446655440000:vault_primary:User vault storage initialized:a7b39f482d8b5c90...
```

### 3.2 Current Hash Computation
The current hash of block $i$ is calculated using libsodium's hardware-accelerated SHA-256:
$$\text{CurrentHash}_i = \text{SHA256}(\text{CanonicalPayload}_i)$$

### 3.3 The Avalanche Effect & Cascading Linkage
Because each block includes $\text{CurrentHash}_{i-1}$ inside its own payload:
* If an attacker modifies even a single character in Block #1, its hash $\text{CurrentHash}_1'$ changes completely (due to the SHA-256 Avalanche Effect).
* Because Block #2 stores the old $\text{CurrentHash}_1$ in its $\text{PreviousHash}_2$ field, the link between Block #1 and Block #2 **breaks immediately**.
* To forge Block #1, the attacker must re-hash Block #2, which changes $\text{CurrentHash}_2$, forcing them to re-hash Block #3, all the way to the newest block in the database!

---

## 4. Audit Event Catalog & Zero-Leakage Policy

BlockVault logs all lifecycle state transitions while guaranteeing that **no secret data is ever written to logs or audit records**:

| Event Type | Trigger Condition | Recorded Entity ID | Recorded Metadata Details |
| :--- | :--- | :--- | :--- |
| `GENESIS` | Database initialization | System UUID | Initialized ledger message |
| `VAULT_CREATED` | New user vault created | Vault UUID | "User vault storage initialized" |
| `USER_REGISTERED` | User account registration | User UUID | "Registered account handle" |
| `USER_LOGIN_SUCCESS` | Authentication success | User UUID | "Successful authentication from {IP}" |
| `USER_LOGIN_FAILED` | Bad password attempt | User UUID | "Failed login attempt from {IP}" |
| `VAULT_UNLOCKED` | Memory enclave unlocked | User UUID | "Enclave unlocked with decrypted DEK" |
| `VAULT_LOCKED` | Session ended / locked | User UUID | "User session ended; memory enclave wiped" |
| `CREDENTIAL_ADDED` | New credential stored | Credential UUID | "Added credential: {Website / Label}" |
| `CREDENTIAL_READ` | Credential accessed | Credential UUID | "Accessed credential details for {Label}" |
| `CREDENTIAL_UPDATED` | Existing credential edited | Credential UUID | "Updated field: {Field Name}" |
| `CREDENTIAL_DELETED` | Credential deleted | Credential UUID | "Deleted credential record" |
| `INTEGRITY_VERIFIED` | Full ledger verification run| User / Admin ID | "All {N} records verified successfully" |

> [!IMPORTANT]
> **Strict Zero-Knowledge Policy:** Under no circumstance does BlockVault record passwords, master passwords, decryption keys, plaintexts, or nonces in the `event_metadata` column. Only non-sensitive labels (e.g. `"GitHub"`, `"AWS Root"`) and opaque UUIDs are recorded.

---

## 5. Cryptographic Verification Engine & Tamper Detection

The integrity engine [`AuditLedger::verify_integrity()`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/src/ledger/AuditLedger.cpp#L68-L135) iterates through the persistent SQLite ledger and applies a 5-step mathematical verification pipeline:

```
[Fetch All Records from SQLite Ordered by record_id ASC]
                         │
                         ▼
             [Verify Genesis Record #0]
           - Index == 0?
           - PrevHash == 000...000?
           - CurrentHash == SHA256(Payload)?
                         │ YES
                         ▼
             [Iterate i from 1 to N-1]
                         │
       ┌─────────────────┴─────────────────┐
       ▼                                   ▼
 [Sequence Continuity]            [Broken Hash Link Check]
 Index[i] == Index[i-1] + 1?      PrevHash[i] == CurrentHash[i-1]?
 (Detects Deleted Records)        (Detects Chain Splices)
       │ YES                               │ YES
       └─────────────────┬─────────────────┘
                         ▼
             [Timestamp Chronology Check]
             Timestamp[i] >= Timestamp[i-1]?
             (Detects Backdated Inserts)
                         │ YES
                         ▼
             [Payload Integrity Check]
             CurrentHash[i] == SHA256(Payload[i])?
             (Detects Tampered Content/Metadata)
                         │ YES
                         ▼
           [RETURN: Ledger Valid & Intact]
```

---

## 6. Automated Test Suite & Tamper Scenarios

The dedicated test harness [`tests/test_audit_ledger.cpp`](file:///c:/Users/DINESHBABU/password%20man/cpp%20project/BlockVault/tests/test_audit_ledger.cpp) verifies six specific security scenarios:

```
========================================================
 BLOCKVAULT C++20 AUDIT LEDGER TEST SUITE
========================================================

[AUDIT TEST] test_untouched_ledger_verification ... PASSED
[AUDIT TEST] test_modify_existing_record ... PASSED
[AUDIT TEST] test_delete_record ... PASSED
[AUDIT TEST] test_change_previous_hash ... PASSED
[AUDIT TEST] test_change_event_metadata ... PASSED
[AUDIT TEST] test_rehash_cascade_detection ... PASSED

--------------------------------------------------------
 AUDIT LEDGER RESULTS: 6 Passed, 0 Failed
========================================================
```

### Verified Scenarios:
1. **Untouched Ledger Verification:** Verifies that a legitimate ledger with multiple lifecycle events passes with `is_valid = true` and `tampered_index = 0`.
2. **Direct Record Content Tampering:** Modifies `event_type` of record #2 via raw SQL (`UPDATE audit_records SET event_type = 'VAULT_UNLOCKED' WHERE record_id = 2`). The verification engine detects `is_valid = false` at index 2 because the recomputed SHA-256 does not match the stored hash.
3. **Record Deletion (Missing Sequence Gap):** Deletes record #2 via SQL (`DELETE FROM audit_records WHERE record_id = 2`). The engine flags `is_valid = false` at index 3, identifying the gap between index 1 and index 3.
4. **Altered Previous Hash:** Corrupts `previous_hash` of record #3. The engine flags `is_valid = false` at index 3 due to a broken hash link.
5. **Modified Event Metadata:** Alters descriptive text in `event_metadata`. Recomputing the canonical payload produces a mismatched SHA-256, caught immediately.
6. **Re-Hash Cascade Detection:** An attacker modifies record #2's metadata **and** recomputes record #2's `current_hash` so record #2 appears internally valid. The verification engine catches the tampering at **record #3** because record #3's `previous_hash` still points to the old hash of record #2!

---

## 7. Limitations of Local Hash Chains & Independent Anchoring

While a local hash chain provides strong mathematical protection against simple tampering, sound security engineering requires understanding its inherent limitations.

### 7.1 The Threat of Local History Rewriting
* **The Root Cause:** In a local single-file database (`vault.db`), the database and the ledger live on the same physical filesystem.
* **The Attack:** If an attacker gains administrator or root access to the host machine, they have read/write access to `vault.db`. An attacker can modify record #2, recalculate `current_hash` for #2, update `previous_hash` in #3, recalculate #3, and repeat all the way to the tip of the ledger ($N$).
* Because the attacker controls all local data, the forged ledger would appear **100% mathematically valid** when verified locally.

### 7.2 Truncation & State Rollback Attacks
* An attacker could delete the latest 10 records from `audit_records`.
* Because the remaining records $0 \dots (N-10)$ are internally consistent and unbroken, local verification cannot know that newer records ever existed unless an external reference exists.

### 7.3 Solutions: External Notarization & Anchoring Protocols

To convert a local tamper-evident ledger into an **independently verifiable audit proof**, BlockVault's architecture supports **Independent Hash Anchoring**:

```
+─────────────────────────────────────────────────────────────────────────────────────────────+
|                               INDEPENDENT HASH ANCHORING MODES                              |
|                                                                                             |
|   BlockVault Local Ledger                                                                   |
|   [Block #0] ──► [Block #1] ──► ... ──► [Block #N (Tip Hash)]                               |
|                                                  │                                          |
|                         ┌────────────────────────┼────────────────────────┐                 |
|                         ▼                        ▼                        ▼                 |
|                  [Mode A: RFC 3161]       [Mode B: WORM Cloud]     [Mode C: Git Tag]        |
|                  Trusted Timestamp        AWS S3 Object Lock       Signed Commit / Push     |
|                  Authority (DigiCert)     (Compliance Mode)        (Protected Branch)       |
+─────────────────────────────────────────────────────────────────────────────────────────────+
```

1. **RFC 3161 Trusted Timestamping Authorities (TSA):**
   - At periodic intervals (or upon vault lock), the application sends the latest tip hash ($\text{CurrentHash}_N$) to a public TSA (e.g. DigiCert or FreeTSA).
   - The TSA returns a cryptographically signed Timestamp Token (TST) containing the hash and trusted atomic clock time.
   - Even with full root access, an attacker cannot forge the TSA's digital signature.
2. **Cloud Write-Once-Read-Many (WORM) Storage:**
   - Periodically append the latest tip hash to an AWS S3 bucket configured with **S3 Object Lock in Compliance Mode** or Azure Immutable Blob Storage.
   - In Compliance Mode, neither the user, the database administrator, nor even AWS root account credentials can delete or overwrite the object until the retention period expires.
3. **Signed Git Commit Anchoring:**
   - Mirror ledger tip hashes into a remote Git repository on a branch protected by branch protection rules and GPG commit signatures.
4. **Public Blockchain Anchoring:**
   - Store the daily tip hash or Merkle root into an `OP_RETURN` payload on a public blockchain (Bitcoin, Ethereum, Polygon).
   - This provides permanent, globally distributed, timestamped proof that the ledger existed in that exact state at that exact point in time—without leaking any user secrets.

