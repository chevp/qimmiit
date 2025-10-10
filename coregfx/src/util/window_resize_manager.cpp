#include <coregfx/util/window_resize_manager.hpp>
#include <coregfx/core/ocean_log.hpp>

namespace coregfx {
namespace platform {

VkResult WindowResizeManager::handleWindowResize(const ResizeConfig& config) {
#ifndef NDEBUG
    ocean::info("=== WINDOW RESIZE START ===");
    ocean::info("New dimensions: " + std::to_string(config.newWidth) + "x" + std::to_string(config.newHeight));
#endif

    if (!validateResizeDimensions(config.newWidth, config.newHeight)) {
        ocean::error("Invalid resize dimensions");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Call pre-resize callback
    if (preResizeCallback) {
#ifndef NDEBUG
        ocean::info("Calling pre-resize callback");
#endif
        preResizeCallback(config.newWidth, config.newHeight);
    }

    // Wait for device to be idle before recreation
    VkResult result = vkDeviceWaitIdle(config.device);
    if (result != VK_SUCCESS) {
        ocean::error("Failed to wait for device idle during resize. VkResult: " + std::to_string(result));
        return result;
    }

    // Recreate swapchain
    result = recreateSwapchain(config);
    if (result != VK_SUCCESS) {
        ocean::error("Failed to recreate swapchain during resize");
        return result;
    }

    // Update related components
    updateFramebuffers(config.newWidth, config.newHeight);
    updateViewport(config.newWidth, config.newHeight);

    // Call post-resize callback
    if (postResizeCallback) {
#ifndef NDEBUG
        ocean::info("Calling post-resize callback");
#endif
        postResizeCallback(config.newWidth, config.newHeight);
    }

#ifndef NDEBUG
    ocean::info("Window resize completed successfully");
    ocean::info("=== WINDOW RESIZE COMPLETE ===");
#endif
    return VK_SUCCESS;
}

bool WindowResizeManager::needsResize(uint32_t currentWidth, uint32_t currentHeight,
                                     uint32_t newWidth, uint32_t newHeight) {
    return (currentWidth != newWidth) || (currentHeight != newHeight);
}

bool WindowResizeManager::validateResizeDimensions(uint32_t width, uint32_t height) {
    // Minimum reasonable window size
    const uint32_t MIN_SIZE = 1;
    const uint32_t MAX_SIZE = 16384; // 16K resolution

    if (width < MIN_SIZE || height < MIN_SIZE) {
        ocean::error("Window dimensions too small: " + std::to_string(width) + "x" + std::to_string(height));
        return false;
    }

    if (width > MAX_SIZE || height > MAX_SIZE) {
        ocean::error("Window dimensions too large: " + std::to_string(width) + "x" + std::to_string(height));
        return false;
    }

    return true;
}

VkResult WindowResizeManager::recreateSwapchain(const ResizeConfig& config) {
#ifndef NDEBUG
    ocean::info("Recreating swapchain for new dimensions");
#endif

    // Query surface capabilities for new size
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(config.physicalDevice, config.surface, &surfaceCapabilities);
    if (result != VK_SUCCESS) {
        ocean::error("Failed to get surface capabilities. VkResult: " + std::to_string(result));
        return result;
    }

    // Validate dimensions against surface capabilities
    uint32_t clampedWidth = std::max(surfaceCapabilities.minImageExtent.width,
                                    std::min(surfaceCapabilities.maxImageExtent.width, config.newWidth));
    uint32_t clampedHeight = std::max(surfaceCapabilities.minImageExtent.height,
                                     std::min(surfaceCapabilities.maxImageExtent.height, config.newHeight));

    if (clampedWidth != config.newWidth || clampedHeight != config.newHeight) {
        ocean::warn("Requested dimensions clamped to surface limits: " +
                   std::to_string(clampedWidth) + "x" + std::to_string(clampedHeight));
    }

    // TODO: Implement actual swapchain recreation
    // This would require access to swapchain creation parameters
#ifndef NDEBUG
    ocean::info("Swapchain recreation logic would be implemented here");
#endif

    return VK_SUCCESS;
}

void WindowResizeManager::updateFramebuffers(uint32_t width, uint32_t height) {
#ifndef NDEBUG
    ocean::info("Updating framebuffers for new dimensions: " + std::to_string(width) + "x" + std::to_string(height));
#endif
    // TODO: Implement framebuffer recreation
    // This would typically involve recreating depth buffers and framebuffers
}

void WindowResizeManager::updateViewport(uint32_t width, uint32_t height) {
#ifndef NDEBUG
    ocean::info("Updating viewport for new dimensions: " + std::to_string(width) + "x" + std::to_string(height));
#endif
    // TODO: Implement viewport updates
    // This would typically involve updating camera projection matrices
}

}} // namespace coregfx::platform