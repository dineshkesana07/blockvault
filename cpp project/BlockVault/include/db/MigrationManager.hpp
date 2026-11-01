#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct sqlite3;

namespace blockvault {

struct MigrationRecord {
    int         version{0};
    std::string description;
    int64_t     applied_at{0};
};

/**
 * @class MigrationManager
 * @brief Schema versioning and migration engine for BlockVault.
 * 
 * Tracks applied database schema versions inside a dedicated schema_migrations
 * table and applies sequential DDL migrations atomically inside transactions.
 */
class MigrationManager {
public:
    MigrationManager() = default;
    ~MigrationManager() = default;

    // Apply all pending migrations in ascending version order
    static int apply_all(sqlite3* db);

    // Retrieve current database schema version (0 if brand new)
    [[nodiscard]] static int get_current_version(sqlite3* db);

    // Retrieve all recorded migrations
    [[nodiscard]] static std::vector<MigrationRecord> get_history(sqlite3* db);
};

} // namespace blockvault

