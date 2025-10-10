#pragma once

#include "StorageBackend.hpp"
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <type_traits>
#include <cstring>

namespace akutik {

/**
 * @brief High-level storage interface
 *
 * Provides a type-safe interface for storing and retrieving data
 * using various storage backends.
 */
class Storage {
public:
    /**
     * @brief Create a storage instance with the default SQLite backend
     * @param connectionString Path to database file
     */
    explicit Storage(const std::string& connectionString);

    /**
     * @brief Create a storage instance with a specific backend
     * @param backend Storage backend to use
     * @param connectionString Backend-specific connection string
     */
    Storage(std::unique_ptr<IStorageBackend> backend, const std::string& connectionString);

    /**
     * @brief Factory method to create storage with specific backend type
     * @param type Backend type to create
     * @param connectionString Backend-specific connection string
     * @return Unique pointer to Storage instance
     */
    static std::unique_ptr<Storage> create(BackendType type, const std::string& connectionString = "");

    ~Storage();

    // Disable copy, allow move
    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage(Storage&&) = default;
    Storage& operator=(Storage&&) = default;

    /**
     * @brief Store a value with a key
     * @tparam T Type of value to store (must be trivially copyable or std::string)
     * @param key Unique identifier
     * @param value Value to store
     * @return true if storage succeeded
     */
    template<typename T>
    bool put(const std::string& key, const T& value);

    /**
     * @brief Retrieve a value by key
     * @tparam T Type of value to retrieve
     * @param key Unique identifier
     * @return Optional containing the value if found
     */
    template<typename T>
    std::optional<T> get(const std::string& key);

    /**
     * @brief Check if a key exists
     * @param key Unique identifier to check
     * @return true if the key exists
     */
    bool exists(const std::string& key);

    /**
     * @brief Remove a key and its associated value
     * @param key Unique identifier to remove
     * @return true if removal succeeded
     */
    bool remove(const std::string& key);

    /**
     * @brief Get all keys matching a pattern
     * @param pattern Pattern to match
     * @return Vector of matching keys
     */
    std::vector<std::string> keys(const std::string& pattern = "*");

    /**
     * @brief Begin a transaction
     * @return true if transaction started successfully
     */
    bool beginTransaction();

    /**
     * @brief Commit the current transaction
     * @return true if commit succeeded
     */
    bool commit();

    /**
     * @brief Rollback the current transaction
     * @return true if rollback succeeded
     */
    bool rollback();

private:
    std::unique_ptr<IStorageBackend> backend_;
};

// Template implementations

template<typename T>
bool Storage::put(const std::string& key, const T& value) {
    std::vector<uint8_t> data;

    if constexpr (std::is_same_v<T, std::string>) {
        // Handle string specially
        data.resize(value.size());
        std::memcpy(data.data(), value.data(), value.size());
    } else if constexpr (std::is_trivially_copyable_v<T>) {
        // Handle POD types
        data.resize(sizeof(T));
        std::memcpy(data.data(), &value, sizeof(T));
    } else {
        static_assert(std::is_trivially_copyable_v<T> || std::is_same_v<T, std::string>,
                      "Type must be trivially copyable or std::string");
        return false;
    }

    return backend_->put(key, data);
}

template<typename T>
std::optional<T> Storage::get(const std::string& key) {
    auto data = backend_->get(key);
    if (!data) {
        return std::nullopt;
    }

    if constexpr (std::is_same_v<T, std::string>) {
        // Handle string specially
        return std::string(reinterpret_cast<const char*>(data->data()), data->size());
    } else if constexpr (std::is_trivially_copyable_v<T>) {
        // Handle POD types
        if (data->size() != sizeof(T)) {
            return std::nullopt; // Size mismatch
        }
        T value;
        std::memcpy(&value, data->data(), sizeof(T));
        return value;
    } else {
        static_assert(std::is_trivially_copyable_v<T> || std::is_same_v<T, std::string>,
                      "Type must be trivially copyable or std::string");
        return std::nullopt;
    }
}

} // namespace akutik