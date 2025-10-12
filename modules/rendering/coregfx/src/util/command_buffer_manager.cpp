#include <coregfx/util/command_buffer_manager.hpp>
#include <coregfx/core/ocean_log.hpp>

namespace coregfx {
namespace platform {

VkResult CommandBufferManager::createCommandPool(const CommandPoolConfig& config, VkCommandPool* commandPool) {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = config.flags;
    poolInfo.queueFamilyIndex = config.queueFamilyIndex;

    VkResult result = vkCreateCommandPool(config.device, &poolInfo, nullptr, commandPool);
    if (result == VK_SUCCESS) {
#ifndef NDEBUG
        ocean::info("Command pool created successfully");
#endif
    } else {
        ocean::error("Failed to create command pool. VkResult: " + std::to_string(result));
    }

    return result;
}

void CommandBufferManager::destroyCommandPool(VkDevice device, VkCommandPool commandPool) {
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, commandPool, nullptr);
    }
}

VkResult CommandBufferManager::createCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                   uint32_t bufferCount, std::vector<VkCommandBuffer>& commandBuffers) {
    commandBuffers.resize(bufferCount);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = bufferCount;

    VkResult result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
    if (result == VK_SUCCESS) {
#ifndef NDEBUG
        ocean::info("Created " + std::to_string(bufferCount) + " command buffers");
#endif
    } else {
        ocean::error("Failed to allocate command buffers. VkResult: " + std::to_string(result));
    }

    return result;
}

void CommandBufferManager::freeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                             std::vector<VkCommandBuffer>& commandBuffers) {
    if (!commandBuffers.empty()) {
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }
}

VkResult CommandBufferManager::beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool,
                                                      VkCommandBuffer* commandBuffer) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffer);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    return vkBeginCommandBuffer(*commandBuffer, &beginInfo);
}

VkResult CommandBufferManager::endSingleTimeCommands(VkDevice device, VkCommandPool commandPool,
                                                    VkQueue queue, VkCommandBuffer commandBuffer) {
    VkResult result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result == VK_SUCCESS) {
        vkQueueWaitIdle(queue);
    }

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    return result;
}

VkResult CommandBufferManager::allocateCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                     VkCommandBufferLevel level, uint32_t bufferCount,
                                                     VkCommandBuffer* commandBuffers) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = bufferCount;

    return vkAllocateCommandBuffers(device, &allocInfo, commandBuffers);
}

}} // namespace coregfx::platform