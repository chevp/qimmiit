/**
 * Cryo Engine - Memory Manager Implementation (stub)
 */

#include "MemoryManager.hpp"

namespace cryo {
namespace engine {

bool MemoryManager::initialize(size_t poolSizeMB) {
    poolSize_ = poolSizeMB * 1024 * 1024;  // Convert to bytes
    return true;
}

void MemoryManager::shutdown() {
    pool_.reset();
}

void* MemoryManager::allocate(size_t size, size_t alignment) {
    // TODO: Implement custom allocation
    allocated_ += size;
    if (allocated_ > peakAllocated_) {
        peakAllocated_ = allocated_;
    }
    return ::operator new(size);
}

void MemoryManager::free(void* ptr) {
    // TODO: Implement custom deallocation
    ::operator delete(ptr);
}

size_t MemoryManager::getTotalAllocated() const {
    return allocated_;
}

size_t MemoryManager::getPeakAllocated() const {
    return peakAllocated_;
}

size_t MemoryManager::getAllocationCount() const {
    return allocations_.size();
}

} // namespace engine
} // namespace cryo