/**
 * Nanook Engine - Memory Manager
 */

#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <mutex>

namespace cryo {
namespace engine {

class MemoryManager {
public:
    bool initialize(size_t poolSizeMB);
    void shutdown();

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));
    void free(void* ptr);

    size_t getTotalAllocated() const;
    size_t getPeakAllocated() const;
    size_t getAllocationCount() const;

private:
    struct AllocationHeader {
        size_t size;
        void* ptr;
    };

    std::unique_ptr<uint8_t[]> pool_;
    size_t poolSize_ = 0;
    size_t allocated_ = 0;
    size_t peakAllocated_ = 0;
    mutable std::mutex mutex_;
    std::vector<AllocationHeader> allocations_;
};

} // namespace engine
} // namespace cryo
