# BlockVault — REST API Specification & Contract Document
**Version:** 1.0.0-PROD  
**Protocol:** HTTP/1.1 over TLS 1.3  
**Content-Type:** `application/json; charset=utf-8`  
**Authentication Scheme:** `Authorization: Bearer <session_token>`

---

## 1. Global API Conventions & Error Contracts

### 1.1 Standard Request Headers
```http
Content-Type: application/json
Accept: application/json
Authorization: Bearer <64_char_hex_session_token>  [Required for protected routes]
X-Client-Node: SV-EUS-0914 (MTLS)                  [Optional client telemetry]
```

### 1.2 Standard Success Envelope
```json
{
  "success": true,
  "timestamp": 1730376000,
  "data": { ... }
}
```

### 1.3 Standard Error Envelope
```json
{
  "success": false,
  "timestamp": 1730376000,
  "error": {
    "code": "AUTH_INVALID_CREDENTIALS",
    "message": "The master passphrase verification check failed.",
    "details": "Argon2id auth verifier mismatch."
  }
}
```

### 1.4 HTTP Status Codes Used
| Status Code | Description | Scenario |
|---|---|---|
| `200 OK` | Request succeeded | Retrieval, update, or verification completed |
| `201 Created` | Resource created | Credential stored, user registered, session initiated |
| `400 Bad Request` | Invalid payload | Schema violation, missing required parameters |
| `401 Unauthorized` | Missing / invalid token | Expired session, missing Bearer token, bad auth |
| `403 Forbidden` | Access denied | Attempting to access another user's vault items |
| `404 Not Found` | Resource missing | Credential ID or user not found |
| `429 Too Many Requests` | Rate limit tripped | Brute-force protection on `/auth/login` triggered |
| `500 Internal Error` | Server fault | Database lock timeout, internal crypto failure |

---

## 2. Authentication Endpoints

### 2.1 Register User & Vault
- **Route:** `POST /api/v1/auth/register`
- **Auth:** None (Public)
- **Description:** Initializes an account, generates cryptographic salt, and stores master auth verifier.

#### Request Body
```json
{
  "email": "elena.vance@sovereign-sec.io",
  "master_password": "StringWithHighEntropy2026!#",
  "vault_name": "Personal Enclave"
}
```

#### Response: `201 Created`
```json
{
  "success": true,
  "timestamp": 1730376010,
  "data": {
    "user_id": "usr_94bf12a0e4c2",
    "email": "elena.vance@sovereign-sec.io",
    "kdf_salt": "a8f3b20c91e4f7a2d1045b89c3214e67",
    "argon_memory_kb": 65536,
    "argon_iterations": 4,
    "created_at": 1730376010
  }
}
```

---

### 2.2 Sign In & Session Creation
- **Route:** `POST /api/v1/auth/login`
- **Auth:** None (Public)
- **Description:** Verifies master passphrase against constant-time Argon2id verifier; returns session token.

#### Request Body
```json
{
  "email": "elena.vance@sovereign-sec.io",
  "master_password": "StringWithHighEntropy2026!#"
}
```

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376045,
  "data": {
    "session_token": "98f4e2c1a0b3d5e789045612347890abcef1234567890abcdef1234567890abc",
    "expires_at": 1730379645,
    "user": {
      "id": "usr_94bf12a0e4c2",
      "email": "elena.vance@sovereign-sec.io",
      "vault_id": "vlt_018274a10e4b"
    }
  }
}
```

---

### 2.3 Terminate Session (Logout / Lock)
- **Route:** `POST /api/v1/auth/logout`
- **Auth:** Bearer Token
- **Description:** Invalidates session token immediately, purges volatile memory caches.

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376090,
  "data": {
    "message": "Session destroyed. Cryptographic state purged."
  }
}
```

---

## 3. Credential Management Endpoints

### 3.1 List All Credentials (Metadata Only)
- **Route:** `GET /api/v1/vault/credentials`
- **Auth:** Bearer Token
- **Query Parameters:**
  - `category` (optional): `logins`, `keys`, `notes`, `ssh`, `cards`
  - `search` (optional): String substring match on title/username/tags
  - `favorites_only` (optional): `true` / `false`

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376120,
  "data": {
    "total": 2,
    "items": [
      {
        "id": "c_1730376001",
        "title": "GitHub Enterprise // DevOps",
        "category": "logins",
        "username": "elena.vance@sovereign-sec.io",
        "url": "https://github.com/sovereign-sec",
        "tags": ["Production", "MFA", "Enclave"],
        "is_favorite": true,
        "is_breached": false,
        "created_at": 1730376001,
        "last_modified": 1730376001
      },
      {
        "id": "c_1730376002",
        "title": "AWS Root IAM // GovCloud",
        "category": "logins",
        "username": "root-iam-0914@sovereign-sec.io",
        "url": "https://signin.aws.amazon.com/console",
        "tags": ["Cloud", "Production"],
        "is_favorite": true,
        "is_breached": false,
        "created_at": 1730376002,
        "last_modified": 1730376002
      }
    ]
  }
}
```

---

### 3.2 Store New Credential
- **Route:** `POST /api/v1/vault/credentials`
- **Auth:** Bearer Token
- **Description:** Encrypts sensitive secret payload with user's active key using XChaCha20-Poly1305 and stores ciphertext + nonce.

#### Request Body
```json
{
  "title": "Cloudflare Zero Trust Portal",
  "category": "logins",
  "username": "elena.vance@cloudflare-access",
  "password": "p9$V#mK9!zX@2Lq8_9aT4#eW",
  "url": "https://dash.cloudflare.com",
  "totp_secret": "JBSWY3DPEHPK3PXP",
  "notes": "Access management for edge DNSSEC zones.",
  "tags": ["Edge", "DNS", "WAF"]
}
```

#### Response: `201 Created`
```json
{
  "success": true,
  "timestamp": 1730376150,
  "data": {
    "id": "c_1730376150",
    "title": "Cloudflare Zero Trust Portal",
    "category": "logins",
    "username": "elena.vance@cloudflare-access",
    "created_at": 1730376150
  }
}
```

---

### 3.3 Retrieve Single Credential (Decrypted View)
- **Route:** `GET /api/v1/vault/credentials/:id`
- **Auth:** Bearer Token
- **Description:** Retrieves ciphertext, decrypts inside secure memory using user session key, returns credential.

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376180,
  "data": {
    "id": "c_1730376150",
    "title": "Cloudflare Zero Trust Portal",
    "category": "logins",
    "username": "elena.vance@cloudflare-access",
    "password": "p9$V#mK9!zX@2Lq8_9aT4#eW",
    "url": "https://dash.cloudflare.com",
    "totp_secret": "JBSWY3DPEHPK3PXP",
    "notes": "Access management for edge DNSSEC zones.",
    "tags": ["Edge", "DNS", "WAF"],
    "is_favorite": false,
    "is_breached": false,
    "entropy_bits": 92.4,
    "last_modified": 1730376150
  }
}
```

---

### 3.4 Delete Credential
- **Route:** `DELETE /api/v1/vault/credentials/:id`
- **Auth:** Bearer Token
- **Description:** Permanently purges credential ciphertext from database and appends deletion record to audit ledger.

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376210,
  "data": {
    "id": "c_1730376150",
    "message": "Credential purged from local keystore."
  }
}
```

---

## 4. Password Generator Endpoints

### 4.1 Synthesize Cryptographic Secret
- **Route:** `POST /api/v1/generator/password`
- **Auth:** None (Public / Client Utility)
- **Description:** Generates high-entropy pseudo-random secrets via libsodium CSPRNG (`randombytes_buf`).

#### Request Body
```json
{
  "length": 24,
  "mode": "random",
  "use_uppercase": true,
  "use_lowercase": true,
  "use_digits": true,
  "use_symbols": true,
  "word_count": 5
}
```

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376240,
  "data": {
    "secret": "k8!M@0P#4vQ9$tL7_1xW3%jZ",
    "entropy_bits": 94.2,
    "strength_score": 85,
    "strength_label": "STRONG",
    "crack_time_estimate": "1,400 Centuries"
  }
}
```

---

## 5. Audit Ledger Endpoints

### 5.1 Fetch Audit Chain Records
- **Route:** `GET /api/v1/audit/ledger`
- **Auth:** Bearer Token
- **Query Parameters:** `limit` (default: 50, max: 200)

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376270,
  "data": {
    "chain_head": "#01428",
    "records": [
      {
        "block_id": "#01428",
        "sequence_num": 1428,
        "timestamp": 1730376210,
        "action": "CREDENTIAL_DELETED",
        "category": "VAULT",
        "target": "c_1730376150",
        "client_node": "SV-EUS-0914 (198.51.100.24)",
        "prev_hash": "0x17b3c4892e01b3e9a740f912e84c98762a1",
        "current_hash": "0x8f4c91a02194b6d39e83ca1209b02a9e144",
        "status": "VERIFIED MERKLE PROOF"
      }
    ]
  }
}
```

---

### 5.2 Verify Merkle Chain Integrity
- **Route:** `POST /api/v1/audit/verify`
- **Auth:** Bearer Token
- **Description:** Recomputes every sequential hash from Genesis Block to Head Block in $O(N)$ time. Returns mathematical validation proof.

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376300,
  "data": {
    "is_valid": true,
    "total_blocks_checked": 1428,
    "head_block_id": "#01428",
    "merkle_root": "0x39a1fe48c90b8321e704a291f038cb127e94",
    "verification_duration_ms": 4.2
  }
}
```

---

## 6. Backup & Disaster Recovery Endpoints

### 6.1 Export Encrypted Backup Snapshot
- **Route:** `POST /api/v1/backup/export`
- **Auth:** Bearer Token
- **Description:** Generates an offline `.enc.json` snapshot containing encrypted blobs and checksums.

#### Response: `200 OK`
```json
{
  "success": true,
  "timestamp": 1730376330,
  "data": {
    "version": "1.0.0-PROD",
    "cipher": "AES-256-GCM / XChaCha20-Poly1305",
    "credential_count": 142,
    "exported_at": "2026-10-31T11:45:30Z",
    "sha256_checksum": "0x98f4e2a1b0c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9",
    "payload_b64": "eyJjdXN0b20iOiJ...[encrypted_snapshot]..."
  }
}
```

---

## 7. Technical Mentoring: REST API Design in C++

### 1. What it is:
A stateless HTTP application programming interface built with C++ Crow where resources are identified by standard URIs, manipulated via standard verbs (`GET`, `POST`, `PUT`, `DELETE`), and formatted in standard JSON.

### 2. Why it is required:
Decoupling the frontend (React) from the backend (C++ Crow) via a well-defined REST contract allows independent testing, mobile client extension (Android/iOS), CLI tooling, and zero-downtime client-side updates.

### 3. How Crow handles REST:
In Crow, routes are mapped via macro bindings:
```cpp
CROW_ROUTE(app, "/api/v1/vault/credentials/<string>")
.methods(crow::HTTPMethod::GET)([](const crow::request& req, std::string cred_id) {
    // 1. Extract Bearer token from req.get_header_value("Authorization")
    // 2. Validate session in SessionManager
    // 3. Delegate to VaultService::get_credential(cred_id)
    // 4. Return crow::response(200, json_payload)
});
```

### 4. Alternatives:
- **gRPC / Protocol Buffers:** Superb performance in microservices, but requires complex web proxying (`grpc-web`) for browser clients.
- **GraphQL:** High query flexibility, but complex C++ server implementations and difficult caching compared to standard REST.
- **WebSockets:** Great for bidirectional streaming, but overkill for standard transactional CRUD.

### 5. Advantages & Disadvantages:
- **Advantages:** Universal browser compatibility, intuitive HTTP semantics, stateless horizontal scaling, simple debuggability via curl/Postman.
- **Disadvantages:** JSON serialization overhead (mitigated by fast libraries like `nlohmann/json` or `simdjson`).
