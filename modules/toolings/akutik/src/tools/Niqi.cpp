#include "akutik/tools/Niqi.hpp"
#include <algorithm>

namespace akutik {
namespace tools {

Niqi::Niqi(size_t maxSizeBytes, uint32_t ttlSeconds)
    : maxSizeBytes_(maxSizeBytes)
    , ttlSeconds_(ttlSeconds)
{
    stats_.maxSize = maxSizeBytes;
}

bool Niqi::put(const std::string& key, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Remove expired entries first
    removeExpired();

    // Check if we need to evict
    size_t requiredSize = data.size();
    while (stats_.currentSize + requiredSize > maxSizeBytes_ && !cache_.empty()) {
        evictLRU();
    }

    // Still too large after eviction?
    if (requiredSize > maxSizeBytes_) {
        return false;
    }

    // Remove old entry if exists
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        stats_.currentSize -= it->second.data.size();
        cache_.erase(it);
    }

    // Insert new entry
    CacheEntry entry;
    entry.data = data;
    entry.timestamp = std::chrono::steady_clock::now();
    entry.accessCount = 0;

    cache_[key] = std::move(entry);
    stats_.currentSize += data.size();
    updateLRU(key);

    return true;
}

std::optional<std::vector<uint8_t>> Niqi::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        stats_.misses++;
        return std::nullopt;
    }

    // Check expiration
    if (isExpired(it->second)) {
        cache_.erase(it);
        stats_.misses++;
        return std::nullopt;
    }

    // Update access
    it->second.accessCount++;
    updateLRU(key);
    stats_.hits++;

    return it->second.data;
}

bool Niqi::contains(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }

    return !isExpired(it->second);
}

bool Niqi::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }

    stats_.currentSize -= it->second.data.size();
    cache_.erase(it);

    // Remove from LRU list
    auto lruIt = std::find(lruList_.begin(), lruList_.end(), key);
    if (lruIt != lruList_.end()) {
        lruList_.erase(lruIt);
    }

    return true;
}

void Niqi::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
    lruList_.clear();
    stats_.currentSize = 0;
}

size_t Niqi::removeExpired() {
    // Note: Assumes mutex is already locked by caller
    if (ttlSeconds_ == 0) {
        return 0; // No expiration
    }

    size_t removed = 0;
    auto now = std::chrono::steady_clock::now();

    for (auto it = cache_.begin(); it != cache_.end(); ) {
        if (isExpired(it->second)) {
            stats_.currentSize -= it->second.data.size();
            it = cache_.erase(it);
            removed++;
        } else {
            ++it;
        }
    }

    return removed;
}

Niqi::Statistics Niqi::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

size_t Niqi::getCurrentSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_.currentSize;
}

size_t Niqi::getEntryCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.size();
}

bool Niqi::isExpired(const CacheEntry& entry) const {
    if (ttlSeconds_ == 0) {
        return false; // No expiration
    }

    auto now = std::chrono::steady_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::seconds>(now - entry.timestamp);
    return age.count() >= ttlSeconds_;
}

void Niqi::evictLRU() {
    // Note: Assumes mutex is already locked by caller
    if (lruList_.empty()) {
        return;
    }

    // Remove least recently used (front of list)
    std::string key = lruList_.front();
    lruList_.erase(lruList_.begin());

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        stats_.currentSize -= it->second.data.size();
        cache_.erase(it);
        stats_.evictions++;
    }
}

void Niqi::updateLRU(const std::string& key) {
    // Note: Assumes mutex is already locked by caller

    // Remove from current position
    auto it = std::find(lruList_.begin(), lruList_.end(), key);
    if (it != lruList_.end()) {
        lruList_.erase(it);
    }

    // Add to end (most recently used)
    lruList_.push_back(key);
}

} // namespace tools
} // namespace akutik
