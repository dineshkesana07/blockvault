#include "db/MigrationManager.hpp"
#include "db/PreparedStatement.hpp"
#include "db/Transaction.hpp"
#include "common/Exceptions.hpp"
#include <sqlite3.h>
#include <chrono>

namespace blockvault {

struct MigrationDefinition {
    int         version;
    const char* description;
    const char* ddl;
};

static const MigrationDefinition MIGRATIONS[] = {
    {
        1,
        "Initial Core Schema: users, credentials, and audit_records",
        R"(
            -- Users Table
            CREATE TABLE IF NOT EXISTS users (
                user_id TEXT PRIMARY KEY,
                username TEXT UNIQUE NOT NULL,
                salt BLOB NOT NULL,
                verifier BLOB NOT NULL,
                created_at INTEGER NOT NULL
            );

            -- Credentials Table
            CREATE TABLE IF NOT EXISTS credentials (
                credential_id TEXT PRIMARY KEY,
                user_id TEXT NOT NULL,
                encrypted_website BLOB NOT NULL,
                encrypted_username BLOB NOT NULL,
                encrypted_password BLOB NOT NULL,
                nonce BLOB NOT NULL,
                category TEXT NOT NULL,
                created_at INTEGER NOT NULL,
                updated_at INTEGER NOT NULL,
                FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
            );

            -- Audit Records Table
            CREATE TABLE IF NOT EXISTS audit_records (
                record_id INTEGER PRIMARY KEY AUTOINCREMENT,
                event_type TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                event_metadata TEXT NOT NULL,
                previous_hash TEXT NOT NULL,
                current_hash TEXT NOT NULL
            );

            -- Performance and Query Optimization Indexes
            CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
            CREATE INDEX IF NOT EXISTS idx_credentials_user_id ON credentials(user_id);
            CREATE INDEX IF NOT EXISTS idx_credentials_category ON credentials(category);
            CREATE INDEX IF NOT EXISTS idx_audit_records_timestamp ON audit_records(timestamp);
            CREATE INDEX IF NOT EXISTS idx_audit_records_event_type ON audit_records(event_type);
        )"
    },
    {
        2,
        "Security Session Management: sessions table with foreign key isolation",
        R"(
            -- Sessions Table
            CREATE TABLE IF NOT EXISTS sessions (
                token TEXT PRIMARY KEY,
                user_id TEXT NOT NULL,
                client_ip TEXT NOT NULL,
                expires_at INTEGER NOT NULL,
                FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
            );

            CREATE INDEX IF NOT EXISTS idx_sessions_user_id ON sessions(user_id);
            CREATE INDEX IF NOT EXISTS idx_sessions_expires_at ON sessions(expires_at);
        )"
    }
};

static const size_t MIGRATION_COUNT = sizeof(MIGRATIONS) / sizeof(MIGRATIONS[0]);

int MigrationManager::get_current_version(sqlite3* db) {
    if (!db) return 0;

    // Check if schema_migrations table exists
    const char* check_sql = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='schema_migrations';";
    PreparedStatement stmt_check(db, check_sql);
    if (stmt_check.step() && stmt_check.get_int(0) == 0) {
        return 0; // Table does not exist yet
    }

    const char* version_sql = "SELECT COALESCE(MAX(version), 0) FROM schema_migrations;";
    PreparedStatement stmt_version(db, version_sql);
    if (stmt_version.step()) {
        return stmt_version.get_int(0);
    }
    return 0;
}

int MigrationManager::apply_all(sqlite3* db) {
    if (!db) {
        throw DatabaseException("Cannot apply migrations: database handle is null");
    }

    // Ensure schema_migrations table exists
    const char* init_migrations_table = R"(
        CREATE TABLE IF NOT EXISTS schema_migrations (
            version INTEGER PRIMARY KEY,
            description TEXT NOT NULL,
            applied_at INTEGER NOT NULL
        );
    )";
    char* err_msg = nullptr;
    if (sqlite3_exec(db, init_migrations_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "Unknown error";
        sqlite3_free(err_msg);
        throw DatabaseException("Failed to initialize schema_migrations table: " + err);
    }

    int current_version = get_current_version(db);
    int applied_count = 0;

    for (size_t i = 0; i < MIGRATION_COUNT; ++i) {
        const auto& mig = MIGRATIONS[i];
        if (mig.version > current_version) {
            // Execute migration DDL inside an atomic transaction
            Transaction tx(db);

            char* mig_err = nullptr;
            if (sqlite3_exec(db, mig.ddl, nullptr, nullptr, &mig_err) != SQLITE_OK) {
                std::string err = mig_err ? mig_err : "Unknown error";
                sqlite3_free(mig_err);
                throw DatabaseException("Migration v" + std::to_string(mig.version) + " failed: " + err);
            }

            // Record applied migration
            auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

            const char* record_sql = "INSERT INTO schema_migrations (version, description, applied_at) VALUES (?, ?, ?);";
            PreparedStatement stmt(db, record_sql);
            stmt.bind_int(1, mig.version);
            stmt.bind_text(2, mig.description);
            stmt.bind_int64(3, now_sec);
            stmt.execute();

            tx.commit();
            applied_count++;
        }
    }

    return applied_count;
}

std::vector<MigrationRecord> MigrationManager::get_history(sqlite3* db) {
    std::vector<MigrationRecord> history;
    if (!db) return history;

    const char* sql = "SELECT version, description, applied_at FROM schema_migrations ORDER BY version ASC;";
    try {
        PreparedStatement stmt(db, sql);
        while (stmt.step()) {
            MigrationRecord rec;
            rec.version = stmt.get_int(0);
            rec.description = stmt.get_text(1);
            rec.applied_at = stmt.get_int64(2);
            history.push_back(rec);
        }
    } catch (...) {
        // Table may not exist yet
    }
    return history;
}

} // namespace blockvault

