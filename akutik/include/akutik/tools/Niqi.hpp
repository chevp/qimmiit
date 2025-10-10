#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace akutik {
namespace tools {

/**
 * @brief Niqi - Data ingestion and caching tool
 *
 * Named after the Inuktitut word for "meat/food" (ᓂᕿ),
 * representing nourishment. Niqi ingests raw data and provides
 * caching/feeding functionality to the system.
 *
 * Responsibilities:
 * - In-memory caching with TTL
 * - Cache hit/miss tracking
 * - Automatic expiration
 * - Size-limited LRU caching
 *
 * No business logic - pure caching utility.
 */
class Niqi {
public:
    /**
     * @brief Cache entry with metadata
     */
    struct CacheEntry {
        std::vector<uint8_t> data;
        std::chrono::steady_clock::time_point timestamp;
        size_t accessCount = 0;
    };

    /**
     * @brief Cache statistics
     */
    struct Statistics {
        size_t hits = 0;
        size_t misses = 0;
        size_t evictions = 0;
        size_t currentSize = 0;
        size_t maxSize = 0;
    };

    /**
     * @brief Create a Niqi cache
     * @param maxSizeBytes Maximum cache size in bytes
     * @param ttlSeconds Time-to-live for entries in seconds (0 = infinite)
     */
    explicit Niqi(size_t maxSizeBytes = 100 * 1024 * 1024, // 100 MB default
                  uint32_t ttlSeconds = 3600);  // 1 hour default

    /**
     * @brief Store data in cache
     * @param key Cache key
     * @param data Data to cache
     * @return true if stored successfully
     */
    bool put(const std::string& key, const std::vector<uint8_t>& data);

    /**
     * @brief Retrieve data from cache
     * @param key Cache key
     * @return Optional containing data if found and not expired
     */
    std::optional<std::vector<uint8_t>> get(const std::string& key);

    /**
     * @brief Check if key exists in cache
     * @param key Cache key
     * @return true if key exists and not expired
     */
    bool contains(const std::string& key) const;

    /**
     * @brief Remove entry from cache
     * @param key Cache key
     * @return true if entry was removed
     */
    bool remove(const std::string& key);

    /**
     * @brief Clear all cache entries
     */
    void clear();

    /**
     * @brief Remove expired entries
     * @return Number of entries removed
     */
    size_t removeExpired();

    /**
     * @brief Get cache statistics
     * @return Statistics structure
     */
    Statistics getStatistics() const;

    /**
     * @brief Get current cache size in bytes
     */
    size_t getCurrentSize() const;

    /**
     * @brief Get number of entries in cache
     */
    size_t getEntryCount() const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, CacheEntry> cache_;
    size_t maxSizeBytes_;
    uint32_t ttlSeconds_;
    Statistics stats_;

    // LRU tracking
    std::vector<std::string> lruList_;

    bool isExpired(const CacheEntry& entry) const;
    void evictLRU();
    void updateLRU(const std::string& key);
};

} // namespace tools
} // namespace akutik
