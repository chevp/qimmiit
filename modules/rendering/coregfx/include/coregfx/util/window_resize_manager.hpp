#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <cstdint>

namespace coregfx {
namespace platform {

class WindowResizeManager {
public:
    struct ResizeConfig {
        VkDevice device = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        uint32_t newWidth = 0;
        uint32_t newHeight = 0;
    };

    using ResizeCallback = std::function<void(uint32_t width, uint32_t height)>;

    WindowResizeManager() = default;
    ~WindowResizeManager() = default;

    // Main resize handling
    VkResult handleWindowResize(const ResizeConfig& config);

    // Callback management for custom resize handling
    void setPreResizeCallback(ResizeCallback callback) { preResizeCallback = callback; }
    void setPostResizeCallback(ResizeCallback callback) { postResizeCallback = callback; }

    // Utility methods
    static bool needsResize(uint32_t currentWidth, uint32_t currentHeight,
                           uint32_t newWidth, uint32_t newHeight);

    // Validation
    static bool validateResizeDimensions(uint32_t width, uint32_t height);

private:
    ResizeCallback preResizeCallback;
    ResizeCallback postResizeCallback;

    // Internal resize steps
    VkResult recreateSwapchain(const ResizeConfig& config);
    void updateFramebuffers(uint32_t width, uint32_t height);
    void updateViewport(uint32_t width, uint32_t height);
};

}} // namespace coregfx::platform