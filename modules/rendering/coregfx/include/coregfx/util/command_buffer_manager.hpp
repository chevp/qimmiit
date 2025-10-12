#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace coregfx {
namespace platform {

class CommandBufferManager {
public:
    struct CommandPoolConfig {
        VkDevice device = VK_NULL_HANDLE;
        uint32_t queueFamilyIndex = 0;
        VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    };

    CommandBufferManager() = default;
    ~CommandBufferManager() = default;

    // Command pool management
    VkResult createCommandPool(const CommandPoolConfig& config, VkCommandPool* commandPool);
    void destroyCommandPool(VkDevice device, VkCommandPool commandPool);

    // Command buffer management
    VkResult createCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                 uint32_t bufferCount, std::vector<VkCommandBuffer>& commandBuffers);
    void freeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                           std::vector<VkCommandBuffer>& commandBuffers);

    // Single-use command buffer helpers
    VkResult beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool,
                                    VkCommandBuffer* commandBuffer);
    VkResult endSingleTimeCommands(VkDevice device, VkCommandPool commandPool,
                                  VkQueue queue, VkCommandBuffer commandBuffer);

private:
    // Helper methods
    VkResult allocateCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                   VkCommandBufferLevel level, uint32_t bufferCount,
                                   VkCommandBuffer* commandBuffers);
};

}} // namespace coregfx::platform