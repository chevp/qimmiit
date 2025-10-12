#include <coregfx/util/synchronization_manager.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <coregfx/core/CryoErrorCodes.hpp>

namespace coregfx {
namespace platform {

VkResult SynchronizationManager::createSemaphores(const SemaphoreConfig& config, std::vector<VkSemaphore>& semaphores) {
    semaphores.resize(config.count);

    for (uint32_t i = 0; i < config.count; ++i) {
        VkResult result = createSemaphoreInternal(config.device, &semaphores[i]);
        if (result != VK_SUCCESS) {
            // Set error code for crash diagnosis with VkResult details
            cryo::setCryoErrorCode(cgfx::CRYO_ERROR_VULKAN_SEMAPHORE_FAILED);

            // Clean up any created semaphores
            for (uint32_t j = 0; j < i; ++j) {
                destroySemaphore(config.device, semaphores[j]);
            }
            semaphores.clear();

            // Log detailed error with VkResult name
            ocean::error("Failed to create semaphore " + std::to_string(i) +
                        ". VkResult: " + std::to_string(result) +
                        " (" + std::string(cryo::vkResultToString(result)) + ")");
            return result;
        }
    }

#ifndef NDEBUG
    ocean::info("Created " + std::to_string(config.count) + " semaphores");
#endif
    return VK_SUCCESS;
}

void SynchronizationManager::destroySemaphores(VkDevice device, std::vector<VkSemaphore>& semaphores) {
    for (auto& semaphore : semaphores) {
        destroySemaphore(device, semaphore);
    }
    semaphores.clear();
}

VkResult SynchronizationManager::createFences(const FenceConfig& config, std::vector<VkFence>& fences) {
    fences.resize(config.count);

    for (uint32_t i = 0; i < config.count; ++i) {
        VkResult result = createFenceInternal(config.device, &fences[i], config.signaled);
        if (result != VK_SUCCESS) {
            // Set error code for crash diagnosis with VkResult details
            cryo::setCryoErrorCode(cgfx::CRYO_ERROR_VULKAN_FENCE_FAILED);

            // Clean up any created fences
            for (uint32_t j = 0; j < i; ++j) {
                destroyFence(config.device, fences[j]);
            }
            fences.clear();

            // Log detailed error with VkResult name
            ocean::error("Failed to create fence " + std::to_string(i) +
                        ". VkResult: " + std::to_string(result) +
                        " (" + std::string(cryo::vkResultToString(result)) + ")");
            return result;
        }
    }

#ifndef NDEBUG
    ocean::info("Created " + std::to_string(config.count) + " fences" +
               (config.signaled ? " (signaled)" : " (unsignaled)"));
#endif
    return VK_SUCCESS;
}

void SynchronizationManager::destroyFences(VkDevice device, std::vector<VkFence>& fences) {
    for (auto& fence : fences) {
        destroyFence(device, fence);
    }
    fences.clear();
}

VkResult SynchronizationManager::waitForFences(VkDevice device, const std::vector<VkFence>& fences,
                                              bool waitAll, uint64_t timeout) {
    if (fences.empty()) {
        return VK_SUCCESS;
    }

    return vkWaitForFences(device, static_cast<uint32_t>(fences.size()),
                          fences.data(), waitAll ? VK_TRUE : VK_FALSE, timeout);
}

VkResult SynchronizationManager::resetFences(VkDevice device, const std::vector<VkFence>& fences) {
    if (fences.empty()) {
        return VK_SUCCESS;
    }

    return vkResetFences(device, static_cast<uint32_t>(fences.size()), fences.data());
}

VkResult SynchronizationManager::createSemaphore(VkDevice device, VkSemaphore* semaphore) {
    return createSemaphoreInternal(device, semaphore);
}

VkResult SynchronizationManager::createFence(VkDevice device, VkFence* fence, bool signaled) {
    return createFenceInternal(device, fence, signaled);
}

void SynchronizationManager::destroySemaphore(VkDevice device, VkSemaphore semaphore) {
    if (semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }
}

void SynchronizationManager::destroyFence(VkDevice device, VkFence fence) {
    if (fence != VK_NULL_HANDLE) {
        vkDestroyFence(device, fence, nullptr);
    }
}

VkResult SynchronizationManager::createSemaphoreInternal(VkDevice device, VkSemaphore* semaphore) {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    return vkCreateSemaphore(device, &semaphoreInfo, nullptr, semaphore);
}

VkResult SynchronizationManager::createFenceInternal(VkDevice device, VkFence* fence, bool signaled) {
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signaled) {
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    return vkCreateFence(device, &fenceInfo, nullptr, fence);
}

}} // namespace coregfx::platform