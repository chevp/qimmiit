#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace akutik {

/**
 * @brief Storage backend type enumeration
 */
enum class BackendType {
    SQLite,   // SQLite database backend
    File,     // File-based storage
    Memory    // In-memory storage (for testing)
};

/**
 * @brief Abstract storage backend interface
 *
 * Defines the contract for storage backends that can persist
 * key-value pairs with different data types.
 */
class IStorageBackend {
public:
    virtual ~IStorageBackend() = default;

    /**
     * @brief Initialize the storage backend
     * @param connectionString Backend-specific connection/configuration string
     * @return true if initialization succeeded
     */
    virtual bool initialize(const std::string& connectionString) = 0;

    /**
     * @brief Store a binary blob
     * @param key Unique identifier for the data
     * @param data Binary data to store
     * @return true if storage succeeded
     */
    virtual bool put(const std::string& key, const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Retrieve a binary blob
     * @param key Unique identifier for the data
     * @return Optional containing the data if found
     */
    virtual std::optional<std::vector<uint8_t>> get(const std::string& key) = 0;

    /**
     * @brief Check if a key exists
     * @param key Unique identifier to check
     * @return true if the key exists
     */
    virtual bool exists(const std::string& key) = 0;

    /**
     * @brief Remove data associated with a key
     * @param key Unique identifier to remove
     * @return true if removal succeeded
     */
    virtual bool remove(const std::string& key) = 0;

    /**
     * @brief Get all keys matching a pattern
     * @param pattern Pattern to match (backend-specific syntax)
     * @return Vector of matching keys
     */
    virtual std::vector<std::string> keys(const std::string& pattern = "*") = 0;

    /**
     * @brief Begin a transaction
     * @return true if transaction started successfully
     */
    virtual bool beginTransaction() = 0;

    /**
     * @brief Commit the current transaction
     * @return true if commit succeeded
     */
    virtual bool commit() = 0;

    /**
     * @brief Rollback the current transaction
     * @return true if rollback succeeded
     */
    virtual bool rollback() = 0;

    /**
     * @brief Close the storage backend
     */
    virtual void close() = 0;
};

/**
 * @brief SQLite storage backend implementation
 */
class SQLiteBackend : public IStorageBackend {
public:
    SQLiteBackend();
    ~SQLiteBackend() override;

    bool initialize(const std::string& connectionString) override;
    bool put(const std::string& key, const std::vector<uint8_t>& data) override;
    std::optional<std::vector<uint8_t>> get(const std::string& key) override;
    bool exists(const std::string& key) override;
    bool remove(const std::string& key) override;
    std::vector<std::string> keys(const std::string& pattern = "*") override;
    bool beginTransaction() override;
    bool commit() override;
    bool rollback() override;
    void close() override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace akutik