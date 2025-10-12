#pragma once

#include "../Storage.hpp"
#include <string>
#include <memory>
#include <optional>
#include <functional>

namespace akutik {
namespace tools {

/**
 * @brief IgluStore - Structured storage tool
 *
 * Named after "Iglu" (ᐃᒡᓗ), the traditional Inuit shelter,
 * representing structure and organization. IgluStore builds
 * and manages structured data storage.
 *
 * Responsibilities:
 * - Namespace-based key organization
 * - Batch operations
 * - Transaction support
 * - Query helpers
 *
 * No business logic - pure structured storage utility.
 */
class IgluStore {
public:
    /**
     * @brief Create an IgluStore with a storage backend
     * @param storage Storage backend to use
     */
    explicit IgluStore(std::shared_ptr<Storage> storage);

    /**
     * @brief Store a value in a namespace
     * @tparam T Type of value to store
     * @param namespace_ Namespace for organization
     * @param key Key within namespace
     * @param value Value to store
     * @return true if successful
     */
    template<typename T>
    bool put(const std::string& namespace_, const std::string& key, const T& value) {
        std::string fullKey = makeKey(namespace_, key);
        return storage_->put(fullKey, value);
    }

    /**
     * @brief Retrieve a value from a namespace
     * @tparam T Type of value to retrieve
     * @param namespace_ Namespace
     * @param key Key within namespace
     * @return Optional containing value if found
     */
    template<typename T>
    std::optional<T> get(const std::string& namespace_, const std::string& key) {
        std::string fullKey = makeKey(namespace_, key);
        return storage_->get<T>(fullKey);
    }

    /**
     * @brief Remove a value from a namespace
     * @param namespace_ Namespace
     * @param key Key within namespace
     * @return true if successful
     */
    bool remove(const std::string& namespace_, const std::string& key);

    /**
     * @brief Get all keys in a namespace
     * @param namespace_ Namespace to query
     * @return Vector of keys
     */
    std::vector<std::string> keysInNamespace(const std::string& namespace_);

    /**
     * @brief Remove all keys in a namespace
     * @param namespace_ Namespace to clear
     * @return Number of keys removed
     */
    size_t clearNamespace(const std::string& namespace_);

    /**
     * @brief Begin a transaction
     * @return true if transaction started
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

    /**
     * @brief Execute a batch of operations
     * @param operations Function containing multiple put/get/remove calls
     * @return true if all operations succeeded
     */
    bool batch(std::function<bool()> operations);

    /**
     * @brief Count entries in a namespace
     * @param namespace_ Namespace to count
     * @return Number of entries
     */
    size_t count(const std::string& namespace_);

    /**
     * @brief Check if a namespace exists
     * @param namespace_ Namespace to check
     * @return true if namespace has at least one entry
     */
    bool namespaceExists(const std::string& namespace_);

private:
    std::shared_ptr<Storage> storage_;

    /**
     * @brief Create a full key from namespace and key
     */
    std::string makeKey(const std::string& namespace_, const std::string& key) const;

    /**
     * @brief Split a full key into namespace and key
     */
    std::pair<std::string, std::string> splitKey(const std::string& fullKey) const;
};

} // namespace tools
} // namespace akutik
