#include "akutik/Storage.hpp"
#include "akutik/StorageBackend.hpp"
#include <sqlite3.h>
#include <stdexcept>

namespace akutik {

// SQLiteBackend implementation details
class SQLiteBackend::Impl {
public:
    sqlite3* db = nullptr;
    bool inTransaction = false;

    ~Impl() {
        if (db) {
            sqlite3_close(db);
        }
    }
};

SQLiteBackend::SQLiteBackend() : pImpl(std::make_unique<Impl>()) {}

SQLiteBackend::~SQLiteBackend() {
    close();
}

bool SQLiteBackend::initialize(const std::string& connectionString) {
    int rc = sqlite3_open(connectionString.c_str(), &pImpl->db);
    if (rc != SQLITE_OK) {
        return false;
    }

    // Create storage table
    const char* createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS storage (
            key TEXT PRIMARY KEY,
            value BLOB NOT NULL
        );
        CREATE INDEX IF NOT EXISTS idx_key ON storage(key);
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(pImpl->db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool SQLiteBackend::put(const std::string& key, const std::vector<uint8_t>& data) {
    const char* insertSQL = "INSERT OR REPLACE INTO storage (key, value) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pImpl->db, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 2, data.data(), static_cast<int>(data.size()), SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::optional<std::vector<uint8_t>> SQLiteBackend::get(const std::string& key) {
    const char* selectSQL = "SELECT value FROM storage WHERE key = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pImpl->db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    const void* blobData = sqlite3_column_blob(stmt, 0);
    int blobSize = sqlite3_column_bytes(stmt, 0);

    std::vector<uint8_t> result(blobSize);
    std::memcpy(result.data(), blobData, blobSize);

    sqlite3_finalize(stmt);
    return result;
}

bool SQLiteBackend::exists(const std::string& key) {
    const char* selectSQL = "SELECT 1 FROM storage WHERE key = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pImpl->db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_ROW;
}

bool SQLiteBackend::remove(const std::string& key) {
    const char* deleteSQL = "DELETE FROM storage WHERE key = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pImpl->db, deleteSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::vector<std::string> SQLiteBackend::keys(const std::string& pattern) {
    std::vector<std::string> result;

    // Convert simple * pattern to SQL LIKE pattern
    std::string sqlPattern = pattern;
    if (sqlPattern == "*") {
        sqlPattern = "%";
    } else {
        // Replace * with % for SQL LIKE
        size_t pos = 0;
        while ((pos = sqlPattern.find('*', pos)) != std::string::npos) {
            sqlPattern.replace(pos, 1, "%");
            pos += 1;
        }
    }

    const char* selectSQL = "SELECT key FROM storage WHERE key LIKE ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pImpl->db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return result;
    }

    sqlite3_bind_text(stmt, 1, sqlPattern.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* key = sqlite3_column_text(stmt, 0);
        result.emplace_back(reinterpret_cast<const char*>(key));
    }

    sqlite3_finalize(stmt);
    return result;
}

bool SQLiteBackend::beginTransaction() {
    if (pImpl->inTransaction) {
        return false;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(pImpl->db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    pImpl->inTransaction = true;
    return true;
}

bool SQLiteBackend::commit() {
    if (!pImpl->inTransaction) {
        return false;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(pImpl->db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    pImpl->inTransaction = false;
    return true;
}

bool SQLiteBackend::rollback() {
    if (!pImpl->inTransaction) {
        return false;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(pImpl->db, "ROLLBACK;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    pImpl->inTransaction = false;
    return true;
}

void SQLiteBackend::close() {
    if (pImpl->db) {
        if (pImpl->inTransaction) {
            rollback();
        }
        sqlite3_close(pImpl->db);
        pImpl->db = nullptr;
    }
}

// Storage implementation

Storage::Storage(const std::string& connectionString)
    : backend_(std::make_unique<SQLiteBackend>()) {
    if (!backend_->initialize(connectionString)) {
        throw std::runtime_error("Failed to initialize storage backend");
    }
}

Storage::Storage(std::unique_ptr<IStorageBackend> backend, const std::string& connectionString)
    : backend_(std::move(backend)) {
    if (!backend_->initialize(connectionString)) {
        throw std::runtime_error("Failed to initialize storage backend");
    }
}

std::unique_ptr<Storage> Storage::create(BackendType type, const std::string& connectionString) {
    std::unique_ptr<IStorageBackend> backend;

    switch (type) {
        case BackendType::SQLite:
            backend = std::make_unique<SQLiteBackend>();
            break;
        case BackendType::File:
            throw std::runtime_error("File backend not yet implemented");
        case BackendType::Memory:
            throw std::runtime_error("Memory backend not yet implemented");
        default:
            throw std::runtime_error("Unknown backend type");
    }

    return std::make_unique<Storage>(std::move(backend), connectionString);
}

Storage::~Storage() = default;

bool Storage::exists(const std::string& key) {
    return backend_->exists(key);
}

bool Storage::remove(const std::string& key) {
    return backend_->remove(key);
}

std::vector<std::string> Storage::keys(const std::string& pattern) {
    return backend_->keys(pattern);
}

bool Storage::beginTransaction() {
    return backend_->beginTransaction();
}

bool Storage::commit() {
    return backend_->commit();
}

bool Storage::rollback() {
    return backend_->rollback();
}

} // namespace akutik