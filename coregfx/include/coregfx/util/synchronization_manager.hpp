#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace coregfx {
namespace platform {

class SynchronizationManager {
public:
    struct SemaphoreConfig {
        VkDevice device = VK_NULL_HANDLE;
        uint32_t count = 1;
    };

    struct FenceConfig {
        VkDevice device = VK_NULL_HANDLE;
        uint32_t count = 1;
        bool signaled = true; // Create in signaled state
    };

    SynchronizationManager() = default;
    ~SynchronizationManager() = default;

    // Semaphore management
    VkResult createSemaphores(const SemaphoreConfig& config, std::vector<VkSemaphore>& semaphores);
    void destroySemaphores(VkDevice device, std::vector<VkSemaphore>& semaphores);

    // Fence management
    VkResult createFences(const FenceConfig& config, std::vector<VkFence>& fences);
    void destroyFences(VkDevice device, std::vector<VkFence>& fences);

    // Wait operations
    VkResult waitForFences(VkDevice device, const std::vector<VkFence>& fences,
                          bool waitAll = true, uint64_t timeout = UINT64_MAX);
    VkResult resetFences(VkDevice device, const std::vector<VkFence>& fences);

    // Single object helpers
    VkResult createSemaphore(VkDevice device, VkSemaphore* semaphore);
    VkResult createFence(VkDevice device, VkFence* fence, bool signaled = true);
    void destroySemaphore(VkDevice device, VkSemaphore semaphore);
    void destroyFence(VkDevice device, VkFence fence);

private:
    VkResult createSemaphoreInternal(VkDevice device, VkSemaphore* semaphore);
    VkResult createFenceInternal(VkDevice device, VkFence* fence, bool signaled);
};

}} // namespace coregfx::platform