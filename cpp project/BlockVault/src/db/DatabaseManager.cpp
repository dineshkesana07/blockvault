#include "db/DatabaseManager.hpp"
#include "db/PreparedStatement.hpp"
#include "db/Transaction.hpp"
#include "db/MigrationManager.hpp"
#include "crypto/CryptoManager.hpp"
#include "common/Exceptions.hpp"
#include <sqlite3.h>
#include <cstring>
#include <chrono>

namespace blockvault {

// Helper: Serialize EncryptedPayload into single BLOB: [nonce(24)][tag(16)][ciphertext]
static ByteVector serialize_payload(const EncryptedPayload& payload) {
    ByteVector blob;
    blob.reserve(payload.nonce.size() + payload.tag.size() + payload.ciphertext.size());
    blob.insert(blob.end(), payload.nonce.begin(), payload.nonce.end());
    blob.insert(blob.end(), payload.tag.begin(), payload.tag.end());
    blob.insert(blob.end(), payload.ciphertext.begin(), payload.ciphertext.end());
    return blob;
}

// Helper: Deserialize BLOB into EncryptedPayload
static EncryptedPayload deserialize_payload(const uint8_t* data, size_t size) {
    EncryptedPayload payload;
    if (!data || size < (NONCE_BYTES + MAC_BYTES)) {
        return payload;
    }
    payload.nonce.assign(data, data + NONCE_BYTES);
    payload.tag.assign(data + NONCE_BYTES, data + NONCE_BYTES + MAC_BYTES);
    payload.ciphertext.assign(data + NONCE_BYTES + MAC_BYTES, data + size);
    return payload;
}

DatabaseManager::DatabaseManager() : m_db(nullptr), m_is_open(false) {}

DatabaseManager::DatabaseManager(const std::string& db_path) : DatabaseManager() {
    open(db_path);
}

DatabaseManager::~DatabaseManager() noexcept {
    close();
}

DatabaseManager::DatabaseManager(DatabaseManager&& other) noexcept
    : m_db(other.m_db),
      m_db_path(std::move(other.m_db_path)),
      m_is_open(other.m_is_open) 
{
    other.m_db = nullptr;
    other.m_is_open = false;
}

DatabaseManager& DatabaseManager::operator=(DatabaseManager&& other) noexcept {
    if (this != &other) {
        close();
        m_db = other.m_db;
        m_db_path = std::move(other.m_db_path);
        m_is_open = other.m_is_open;
        other.m_db = nullptr;
        other.m_is_open = false;
    }
    return *this;
}

void DatabaseManager::open(const std::string& db_path) {
    if (m_is_open) {
        close();
    }

    int rc = sqlite3_open_v2(
        db_path.c_str(),
        &m_db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
        nullptr
    );

    if (rc != SQLITE_OK) {
        std::string err = m_db ? sqlite3_errmsg(m_db) : "Unknown error";
        close();
        throw DatabaseException("Failed to open SQLite database at '" + db_path + "': " + err);
    }

    m_db_path = db_path;
    m_is_open = true;

    configure_pragmas();
    apply_migrations();
}

void DatabaseManager::close() noexcept {
    if (m_db) {
        sqlite3_close_v2(m_db);
        m_db = nullptr;
    }
    m_is_open = false;
}

void DatabaseManager::configure_pragmas() {
    // 1. Mandatory Foreign Key enforcement
    execute_raw("PRAGMA foreign_keys = ON;");

    // 2. High-concurrency Write-Ahead Logging (WAL)
    execute_raw("PRAGMA journal_mode = WAL;");

    // 3. Optimized synchronous mode for WAL
    execute_raw("PRAGMA synchronous = NORMAL;");

    // 4. Busy timeout to prevent lock contention crashes
    execute_raw("PRAGMA busy_timeout = 5000;");
}

void DatabaseManager::apply_migrations() {
    MigrationManager::apply_all(m_db);
}

void DatabaseManager::execute_raw(const std::string& sql) {
    if (!is_open()) throw DatabaseException("Cannot execute raw SQL: Database is not open");
    char* err_msg = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "Unknown error";
        sqlite3_free(err_msg);
        throw DatabaseException("SQL execution error: " + err + " [SQL: " + sql + "]");
    }
}

int DatabaseManager::get_schema_version() {
    return MigrationManager::get_current_version(m_db);
}

Transaction DatabaseManager::create_transaction() {
    if (!is_open()) throw DatabaseException("Cannot create transaction: Database is not open");
    return Transaction(m_db);
}

// -----------------------------------------------------------------------------
// Direct Relational Entity CRUD (users, credentials, audit_records)
// -----------------------------------------------------------------------------

// --- 1. Users Table Operations ---

void DatabaseManager::insert_user_entity(const UserEntity& user) {
    const char* sql = "INSERT INTO users (user_id, username, salt, verifier, created_at) "
                      "VALUES (?, ?, ?, ?, ?);";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, user.user_id);
    stmt.bind_text(2, user.username);
    stmt.bind_blob(3, user.salt);
    stmt.bind_blob(4, user.verifier);
    stmt.bind_int64(5, user.created_at);
    stmt.execute();
}

std::optional<UserEntity> DatabaseManager::get_user_entity_by_id(const std::string& user_id) {
    const char* sql = "SELECT user_id, username, salt, verifier, created_at FROM users WHERE user_id = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, user_id);

    if (stmt.step()) {
        UserEntity u;
        u.user_id = stmt.get_text(0);
        u.username = stmt.get_text(1);
        u.salt = stmt.get_blob(2);
        u.verifier = stmt.get_blob(3);
        u.created_at = stmt.get_int64(4);
        return u;
    }
    return std::nullopt;
}

std::optional<UserEntity> DatabaseManager::get_user_entity_by_username(const std::string& username) {
    const char* sql = "SELECT user_id, username, salt, verifier, created_at FROM users WHERE username = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, username);

    if (stmt.step()) {
        UserEntity u;
        u.user_id = stmt.get_text(0);
        u.username = stmt.get_text(1);
        u.salt = stmt.get_blob(2);
        u.verifier = stmt.get_blob(3);
        u.created_at = stmt.get_int64(4);
        return u;
    }
    return std::nullopt;
}

void DatabaseManager::update_user_entity_verifier(
    const std::string& user_id,
    const ByteVector& salt,
    const ByteVector& verifier) 
{
    const char* sql = "UPDATE users SET salt = ?, verifier = ? WHERE user_id = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_blob(1, salt);
    stmt.bind_blob(2, verifier);
    stmt.bind_text(3, user_id);
    stmt.execute();
}

bool DatabaseManager::delete_user_entity(const std::string& user_id) {
    const char* sql = "DELETE FROM users WHERE user_id = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, user_id);
    stmt.execute();
    return sqlite3_changes(m_db) > 0;
}

// --- 2. Credentials Table Operations ---

void DatabaseManager::insert_credential_entity(const CredentialEntity& credential) {
    const char* sql = "INSERT INTO credentials (credential_id, user_id, encrypted_website, "
                      "encrypted_username, encrypted_password, nonce, category, created_at, updated_at) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, credential.credential_id);
    stmt.bind_text(2, credential.user_id);
    stmt.bind_blob(3, credential.encrypted_website);
    stmt.bind_blob(4, credential.encrypted_username);
    stmt.bind_blob(5, credential.encrypted_password);
    stmt.bind_blob(6, credential.nonce);
    stmt.bind_text(7, credential.category);
    stmt.bind_int64(8, credential.created_at);
    stmt.bind_int64(9, credential.updated_at);
    stmt.execute();
}

std::optional<CredentialEntity> DatabaseManager::get_credential_entity_by_id(
    const std::string& credential_id,
    const std::string& user_id) 
{
    const char* sql = "SELECT credential_id, user_id, encrypted_website, encrypted_username, "
                      "encrypted_password, nonce, category, created_at, updated_at "
                      "FROM credentials WHERE credential_id = ? AND user_id = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, credential_id);
    stmt.bind_text(2, user_id);

    if (stmt.step()) {
        CredentialEntity c;
        c.credential_id = stmt.get_text(0);
        c.user_id = stmt.get_text(1);
        c.encrypted_website = stmt.get_blob(2);
        c.encrypted_username = stmt.get_blob(3);
        c.encrypted_password = stmt.get_blob(4);
        c.nonce = stmt.get_blob(5);
        c.category = stmt.get_text(6);
        c.created_at = stmt.get_int64(7);
        c.updated_at = stmt.get_int64(8);
        return c;
    }
    return std::nullopt;
}

std::vector<CredentialEntity> DatabaseManager::get_credential_entities_for_user(const std::string& user_id) {
    std::vector<CredentialEntity> list;
    const char* sql = "SELECT credential_id, user_id, encrypted_website, encrypted_username, "
                      "encrypted_password, nonce, category, created_at, updated_at "
                      "FROM credentials WHERE user_id = ? ORDER BY created_at DESC;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, user_id);

    while (stmt.step()) {
        CredentialEntity c;
        c.credential_id = stmt.get_text(0);
        c.user_id = stmt.get_text(1);
        c.encrypted_website = stmt.get_blob(2);
        c.encrypted_username = stmt.get_blob(3);
        c.encrypted_password = stmt.get_blob(4);
        c.nonce = stmt.get_blob(5);
        c.category = stmt.get_text(6);
        c.created_at = stmt.get_int64(7);
        c.updated_at = stmt.get_int64(8);
        list.push_back(std::move(c));
    }
    return list;
}

void DatabaseManager::update_credential_entity(const CredentialEntity& credential) {
    const char* sql = "UPDATE credentials SET encrypted_website = ?, encrypted_username = ?, "
                      "encrypted_password = ?, nonce = ?, category = ?, updated_at = ? "
                      "WHERE credential_id = ? AND user_id = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_blob(1, credential.encrypted_website);
    stmt.bind_blob(2, credential.encrypted_username);
    stmt.bind_blob(3, credential.encrypted_password);
    stmt.bind_blob(4, credential.nonce);
    stmt.bind_text(5, credential.category);
    stmt.bind_int64(6, credential.updated_at);
    stmt.bind_text(7, credential.credential_id);
    stmt.bind_text(8, credential.user_id);
    stmt.execute();
}

bool DatabaseManager::delete_credential_entity(const std::string& credential_id, const std::string& user_id) {
    const char* sql = "DELETE FROM credentials WHERE credential_id = ? AND user_id = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, credential_id);
    stmt.bind_text(2, user_id);
    stmt.execute();
    return sqlite3_changes(m_db) > 0;
}

size_t DatabaseManager::get_credential_entity_count(const std::string& user_id) {
    const char* sql = "SELECT COUNT(*) FROM credentials WHERE user_id = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, user_id);
    if (stmt.step()) {
        return static_cast<size_t>(stmt.get_int64(0));
    }
    return 0;
}

// --- 3. Audit Records Table Operations ---

int64_t DatabaseManager::insert_audit_record(const AuditRecordEntity& record) {
    const char* sql = "INSERT INTO audit_records (event_type, timestamp, event_metadata, previous_hash, current_hash) "
                      "VALUES (?, ?, ?, ?, ?);";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, record.event_type);
    stmt.bind_int64(2, record.timestamp);
    stmt.bind_text(3, record.event_metadata);
    stmt.bind_text(4, record.previous_hash);
    stmt.bind_text(5, record.current_hash);
    stmt.execute();
    return sqlite3_last_insert_rowid(m_db);
}

std::optional<AuditRecordEntity> DatabaseManager::get_last_audit_record() {
    const char* sql = "SELECT record_id, event_type, timestamp, event_metadata, previous_hash, current_hash "
                      "FROM audit_records ORDER BY record_id DESC LIMIT 1;";
    PreparedStatement stmt(m_db, sql);
    if (stmt.step()) {
        AuditRecordEntity r;
        r.record_id = stmt.get_int64(0);
        r.event_type = stmt.get_text(1);
        r.timestamp = stmt.get_int64(2);
        r.event_metadata = stmt.get_text(3);
        r.previous_hash = stmt.get_text(4);
        r.current_hash = stmt.get_text(5);
        return r;
    }
    return std::nullopt;
}

std::vector<AuditRecordEntity> DatabaseManager::get_all_audit_records() {
    std::vector<AuditRecordEntity> list;
    const char* sql = "SELECT record_id, event_type, timestamp, event_metadata, previous_hash, current_hash "
                      "FROM audit_records ORDER BY record_id ASC;";
    PreparedStatement stmt(m_db, sql);
    while (stmt.step()) {
        AuditRecordEntity r;
        r.record_id = stmt.get_int64(0);
        r.event_type = stmt.get_text(1);
        r.timestamp = stmt.get_int64(2);
        r.event_metadata = stmt.get_text(3);
        r.previous_hash = stmt.get_text(4);
        r.current_hash = stmt.get_text(5);
        list.push_back(std::move(r));
    }
    return list;
}

std::vector<AuditRecordEntity> DatabaseManager::get_audit_records_by_event(const std::string& event_type) {
    std::vector<AuditRecordEntity> list;
    const char* sql = "SELECT record_id, event_type, timestamp, event_metadata, previous_hash, current_hash "
                      "FROM audit_records WHERE event_type = ? ORDER BY record_id ASC;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, event_type);
    while (stmt.step()) {
        AuditRecordEntity r;
        r.record_id = stmt.get_int64(0);
        r.event_type = stmt.get_text(1);
        r.timestamp = stmt.get_int64(2);
        r.event_metadata = stmt.get_text(3);
        r.previous_hash = stmt.get_text(4);
        r.current_hash = stmt.get_text(5);
        list.push_back(std::move(r));
    }
    return list;
}

uint64_t DatabaseManager::get_audit_record_count() {
    const char* sql = "SELECT COUNT(*) FROM audit_records;";
    PreparedStatement stmt(m_db, sql);
    if (stmt.step()) {
        return static_cast<uint64_t>(stmt.get_int64(0));
    }
    return 0;
}

// -----------------------------------------------------------------------------
// High-Level Domain Model Compatibility Adapters (User, Credential, Block)
// -----------------------------------------------------------------------------

void DatabaseManager::insert_user(const User& user) {
    ByteVector hash_bytes;
    if (user.password_hash().length() == 64) {
        hash_bytes = CryptoManager::from_hex(user.password_hash());
    } else {
        hash_bytes.assign(user.password_hash().begin(), user.password_hash().end());
        hash_bytes.resize(32, 0);
    }

    ByteVector dek_blob = serialize_payload(user.encrypted_dek());
    ByteVector verifier_combined;
    verifier_combined.reserve(hash_bytes.size() + dek_blob.size());
    verifier_combined.insert(verifier_combined.end(), hash_bytes.begin(), hash_bytes.end());
    verifier_combined.insert(verifier_combined.end(), dek_blob.begin(), dek_blob.end());

    auto created_sec = std::chrono::duration_cast<std::chrono::seconds>(user.created_at().time_since_epoch()).count();

    UserEntity entity;
    entity.user_id = user.id();
    entity.username = user.email(); // Mapped to username
    entity.salt = user.salt();
    entity.verifier = verifier_combined;
    entity.created_at = created_sec;

    insert_user_entity(entity);
}

std::optional<User> DatabaseManager::get_user_by_id(const std::string& user_id) {
    auto u_opt = get_user_entity_by_id(user_id);
    if (!u_opt.has_value()) return std::nullopt;

    std::string hash = "";
    EncryptedPayload dek;
    if (u_opt->verifier.size() >= 32) {
        hash = CryptoManager::to_hex(u_opt->verifier.data(), 32);
        dek = deserialize_payload(u_opt->verifier.data() + 32, u_opt->verifier.size() - 32);
    } else {
        dek = deserialize_payload(u_opt->verifier.data(), u_opt->verifier.size());
    }
    Timestamp created = Timestamp(std::chrono::seconds(u_opt->created_at));

    return User(
        u_opt->user_id,
        u_opt->username,
        hash,
        u_opt->salt,
        dek,
        created,
        created
    );
}

std::optional<User> DatabaseManager::get_user_by_email(const std::string& email) {
    auto u_opt = get_user_entity_by_username(email);
    if (!u_opt.has_value()) return std::nullopt;

    std::string hash = "";
    EncryptedPayload dek;
    if (u_opt->verifier.size() >= 32) {
        hash = CryptoManager::to_hex(u_opt->verifier.data(), 32);
        dek = deserialize_payload(u_opt->verifier.data() + 32, u_opt->verifier.size() - 32);
    } else {
        dek = deserialize_payload(u_opt->verifier.data(), u_opt->verifier.size());
    }
    Timestamp created = Timestamp(std::chrono::seconds(u_opt->created_at));

    return User(
        u_opt->user_id,
        u_opt->username,
        hash,
        u_opt->salt,
        dek,
        created,
        created
    );
}

void DatabaseManager::update_user(const User& user) {
    ByteVector hash_bytes;
    if (user.password_hash().length() == 64) {
        hash_bytes = CryptoManager::from_hex(user.password_hash());
    } else {
        hash_bytes.assign(user.password_hash().begin(), user.password_hash().end());
        hash_bytes.resize(32, 0);
    }
    ByteVector dek_blob = serialize_payload(user.encrypted_dek());
    ByteVector verifier_combined;
    verifier_combined.reserve(hash_bytes.size() + dek_blob.size());
    verifier_combined.insert(verifier_combined.end(), hash_bytes.begin(), hash_bytes.end());
    verifier_combined.insert(verifier_combined.end(), dek_blob.begin(), dek_blob.end());

    update_user_entity_verifier(user.id(), user.salt(), verifier_combined);
}

bool DatabaseManager::delete_user(const std::string& user_id) {
    return delete_user_entity(user_id);
}

void DatabaseManager::insert_credential(const Credential& credential) {
    ByteVector pass_blob = serialize_payload(credential.encrypted_password());
    ByteVector notes_blob = serialize_payload(credential.encrypted_notes());
    auto created_sec = std::chrono::duration_cast<std::chrono::seconds>(credential.created_at().time_since_epoch()).count();
    auto updated_sec = std::chrono::duration_cast<std::chrono::seconds>(credential.updated_at().time_since_epoch()).count();

    std::string combined_site = credential.title() + "\n" + credential.website_url();
    std::string cat_str = category_to_string(credential.category()) + (credential.is_favorite() ? ":FAV" : "");

    CredentialEntity c;
    c.credential_id = credential.id();
    c.user_id = credential.user_id();
    c.encrypted_website = ByteVector(combined_site.begin(), combined_site.end());
    c.encrypted_username = ByteVector(credential.username().begin(), credential.username().end());
    c.encrypted_password = pass_blob;
    c.nonce = notes_blob; // Holds secondary authenticated notes payload
    c.category = cat_str;
    c.created_at = created_sec;
    c.updated_at = updated_sec;

    insert_credential_entity(c);
}

std::optional<Credential> DatabaseManager::get_credential_by_id(
    const std::string& credential_id,
    const std::string& user_id) 
{
    auto c_opt = get_credential_entity_by_id(credential_id, user_id);
    if (!c_opt.has_value()) return std::nullopt;

    std::string combined(c_opt->encrypted_website.begin(), c_opt->encrypted_website.end());
    std::string title = combined;
    std::string url = combined;
    size_t sep = combined.find('\n');
    if (sep != std::string::npos) {
        title = combined.substr(0, sep);
        url = combined.substr(sep + 1);
    }

    std::string user(c_opt->encrypted_username.begin(), c_opt->encrypted_username.end());
    EncryptedPayload pass = deserialize_payload(c_opt->encrypted_password.data(), c_opt->encrypted_password.size());
    EncryptedPayload notes = deserialize_payload(c_opt->nonce.data(), c_opt->nonce.size());

    bool fav = false;
    std::string cat_str = c_opt->category;
    size_t fav_pos = cat_str.find(":FAV");
    if (fav_pos != std::string::npos) {
        fav = true;
        cat_str = cat_str.substr(0, fav_pos);
    }

    return Credential(
        c_opt->credential_id,
        c_opt->user_id,
        title,
        url,
        user,
        pass,
        notes,
        string_to_category(cat_str),
        fav,
        Timestamp(std::chrono::seconds(c_opt->created_at)),
        Timestamp(std::chrono::seconds(c_opt->updated_at))
    );
}

std::vector<Credential> DatabaseManager::get_credentials_for_user(const std::string& user_id) {
    std::vector<Credential> list;
    auto entities = get_credential_entities_for_user(user_id);
    for (const auto& c_opt : entities) {
        std::string combined(c_opt.encrypted_website.begin(), c_opt.encrypted_website.end());
        std::string title = combined;
        std::string url = combined;
        size_t sep = combined.find('\n');
        if (sep != std::string::npos) {
            title = combined.substr(0, sep);
            url = combined.substr(sep + 1);
        }

        std::string user(c_opt.encrypted_username.begin(), c_opt.encrypted_username.end());
        EncryptedPayload pass = deserialize_payload(c_opt.encrypted_password.data(), c_opt.encrypted_password.size());
        EncryptedPayload notes = deserialize_payload(c_opt.nonce.data(), c_opt.nonce.size());

        bool fav = false;
        std::string cat_str = c_opt.category;
        size_t fav_pos = cat_str.find(":FAV");
        if (fav_pos != std::string::npos) {
            fav = true;
            cat_str = cat_str.substr(0, fav_pos);
        }

        list.emplace_back(
            c_opt.credential_id,
            c_opt.user_id,
            title,
            url,
            user,
            pass,
            notes,
            string_to_category(cat_str),
            fav,
            Timestamp(std::chrono::seconds(c_opt.created_at)),
            Timestamp(std::chrono::seconds(c_opt.updated_at))
        );
    }
    return list;
}

void DatabaseManager::update_credential(const Credential& credential) {
    ByteVector pass_blob = serialize_payload(credential.encrypted_password());
    ByteVector notes_blob = serialize_payload(credential.encrypted_notes());
    auto updated_sec = std::chrono::duration_cast<std::chrono::seconds>(credential.updated_at().time_since_epoch()).count();

    std::string combined_site = credential.title() + "\n" + credential.website_url();
    std::string cat_str = category_to_string(credential.category()) + (credential.is_favorite() ? ":FAV" : "");

    CredentialEntity c;
    c.credential_id = credential.id();
    c.user_id = credential.user_id();
    c.encrypted_website = ByteVector(combined_site.begin(), combined_site.end());
    c.encrypted_username = ByteVector(credential.username().begin(), credential.username().end());
    c.encrypted_password = pass_blob;
    c.nonce = notes_blob;
    c.category = cat_str;
    c.updated_at = updated_sec;

    update_credential_entity(c);
}

bool DatabaseManager::delete_credential(const std::string& credential_id, const std::string& user_id) {
    return delete_credential_entity(credential_id, user_id);
}

size_t DatabaseManager::get_credential_count(const std::string& user_id) {
    return get_credential_entity_count(user_id);
}

void DatabaseManager::insert_block(const Block& block) {
    auto time_sec = std::chrono::duration_cast<std::chrono::seconds>(block.timestamp().time_since_epoch()).count();
    AuditRecordEntity rec;
    rec.event_type = event_type_to_string(block.event_type());
    rec.timestamp = time_sec;
    rec.event_metadata = block.user_id() + ":" + block.entity_id() + ":" + block.details();
    rec.previous_hash = block.previous_hash();
    rec.current_hash = block.current_hash();
    insert_audit_record(rec);
}

std::optional<Block> DatabaseManager::get_last_block() {
    auto r_opt = get_last_audit_record();
    if (!r_opt.has_value()) return std::nullopt;

    // Parse metadata back: user_id:entity_id:details
    std::string meta = r_opt->event_metadata;
    std::string uid = "SYSTEM";
    std::string eid = "";
    std::string det = meta;

    size_t pos1 = meta.find(':');
    if (pos1 != std::string::npos) {
        uid = meta.substr(0, pos1);
        size_t pos2 = meta.find(':', pos1 + 1);
        if (pos2 != std::string::npos) {
            eid = meta.substr(pos1 + 1, pos2 - (pos1 + 1));
            det = meta.substr(pos2 + 1);
        }
    }

    return Block(
        static_cast<uint64_t>(r_opt->record_id - 1), // 0-based block index
        string_to_event_type(r_opt->event_type),
        uid,
        eid,
        det,
        r_opt->previous_hash,
        r_opt->current_hash,
        Timestamp(std::chrono::seconds(r_opt->timestamp))
    );
}

std::vector<Block> DatabaseManager::get_all_blocks() {
    std::vector<Block> list;
    auto records = get_all_audit_records();
    for (const auto& r_opt : records) {
        std::string meta = r_opt.event_metadata;
        std::string uid = "SYSTEM";
        std::string eid = "";
        std::string det = meta;

        size_t pos1 = meta.find(':');
        if (pos1 != std::string::npos) {
            uid = meta.substr(0, pos1);
            size_t pos2 = meta.find(':', pos1 + 1);
            if (pos2 != std::string::npos) {
                eid = meta.substr(pos1 + 1, pos2 - (pos1 + 1));
                det = meta.substr(pos2 + 1);
            }
        }

        list.emplace_back(
            static_cast<uint64_t>(r_opt.record_id - 1),
            string_to_event_type(r_opt.event_type),
            uid,
            eid,
            det,
            r_opt.previous_hash,
            r_opt.current_hash,
            Timestamp(std::chrono::seconds(r_opt.timestamp))
        );
    }
    return list;
}

std::vector<Block> DatabaseManager::get_blocks_for_user(const std::string& user_id) {
    std::vector<Block> all = get_all_blocks();
    std::vector<Block> filtered;
    for (const auto& b : all) {
        if (b.user_id() == user_id || b.user_id() == "SYSTEM") {
            filtered.push_back(b);
        }
    }
    return filtered;
}

uint64_t DatabaseManager::get_block_count() {
    return get_audit_record_count();
}

// -----------------------------------------------------------------------------
// Session Operations
// -----------------------------------------------------------------------------

void DatabaseManager::insert_session(
    const std::string& token,
    const std::string& user_id,
    const std::string& client_ip,
    int64_t expires_at) 
{
    const char* sql = "INSERT OR REPLACE INTO sessions (token, user_id, client_ip, expires_at) "
                      "VALUES (?, ?, ?, ?);";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, token);
    stmt.bind_text(2, user_id);
    stmt.bind_text(3, client_ip);
    stmt.bind_int64(4, expires_at);
    stmt.execute();
}

std::optional<std::string> DatabaseManager::validate_session(const std::string& token) {
    const char* sql = "SELECT user_id, expires_at FROM sessions WHERE token = ?;";
    PreparedStatement stmt(m_db, sql);
    stmt.bind_text(1, token);

    if (stmt.step()) {
        std::string uid = stmt.get_text(0);
        int64_t expires = stmt.get_int64(1);
        auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        if (expires > now_sec) {
            return uid;
        }
        delete_session(token);
    }
    return std::nullopt;
}

void DatabaseManager::delete_session(const std::string& token) {
    if (!is_open()) return;
    try {
        const char* sql = "DELETE FROM sessions WHERE token = ?;";
        PreparedStatement stmt(m_db, sql);
        stmt.bind_text(1, token);
        stmt.execute();
    } catch (...) {}
}

void DatabaseManager::delete_user_sessions(const std::string& user_id) {
    if (!is_open()) return;
    try {
        const char* sql = "DELETE FROM sessions WHERE user_id = ?;";
        PreparedStatement stmt(m_db, sql);
        stmt.bind_text(1, user_id);
        stmt.execute();
    } catch (...) {}
}

void DatabaseManager::purge_expired_sessions() {
    if (!is_open()) return;
    try {
        auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        const char* sql = "DELETE FROM sessions WHERE expires_at <= ?;";
        PreparedStatement stmt(m_db, sql);
        stmt.bind_int64(1, now_sec);
        stmt.execute();
    } catch (...) {}
}

} // namespace blockvault
