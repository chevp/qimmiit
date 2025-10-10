#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace coregfx {
namespace platform {

class FramebufferManager {
public:
    struct FramebufferConfig {
        uint32_t width = 1280;
        uint32_t height = 720;
        VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
        bool useMultisampling = false;
    };

    struct DepthStencil {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
    };

    struct MultisampleTarget {
        struct {
            VkImage image = VK_NULL_HANDLE;
            VkImageView view = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
        } color;
        struct {
            VkImage image = VK_NULL_HANDLE;
            VkImageView view = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
        } depth;
    };

    FramebufferManager() = default;
    ~FramebufferManager() = default;

    // Render pass creation
    VkResult createRenderPass(VkDevice device, const FramebufferConfig& config, VkRenderPass* renderPass);
    VkResult createMultisampleRenderPass(VkDevice device, const FramebufferConfig& config, VkRenderPass* renderPass);

    // Depth buffer management
    VkResult createDepthStencil(VkDevice device, VkPhysicalDevice physicalDevice,
                               const FramebufferConfig& config, DepthStencil* depthStencil);
    void destroyDepthStencil(VkDevice device, DepthStencil* depthStencil);

    // Multisampling targets
    VkResult createMultisampleTarget(VkDevice device, VkPhysicalDevice physicalDevice,
                                    const FramebufferConfig& config, MultisampleTarget* msaaTarget);
    void destroyMultisampleTarget(VkDevice device, MultisampleTarget* msaaTarget);

    // Framebuffer management
    VkResult createFramebuffers(VkDevice device, VkRenderPass renderPass,
                               const std::vector<VkImageView>& swapChainImageViews,
                               const DepthStencil& depthStencil,
                               const FramebufferConfig& config,
                               std::vector<VkFramebuffer>& framebuffers);

    VkResult createMultisampleFramebuffers(VkDevice device, VkRenderPass renderPass,
                                          const std::vector<VkImageView>& swapChainImageViews,
                                          const MultisampleTarget& msaaTarget,
                                          const DepthStencil& depthStencil,
                                          const FramebufferConfig& config,
                                          std::vector<VkFramebuffer>& framebuffers);

    void destroyFramebuffers(VkDevice device, std::vector<VkFramebuffer>& framebuffers);

    // Utility functions
    static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
    static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice,
                                       const std::vector<VkFormat>& candidates,
                                       VkImageTiling tiling,
                                       VkFormatFeatureFlags features);
    static bool hasStencilComponent(VkFormat format);

private:
    VkResult createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                        uint32_t width, uint32_t height, VkFormat format,
                        VkImageTiling tiling, VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties, VkSampleCountFlagBits sampleCount,
                        VkImage* image, VkDeviceMemory* imageMemory);

    VkResult createImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspectFlags, VkImageView* imageView);

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                           VkMemoryPropertyFlags properties);
};

}} // namespace coregfx::platform