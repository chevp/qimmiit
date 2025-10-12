#include <coregfx/util/framebuffer_manager.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <array>

namespace coregfx {
namespace platform {

VkResult FramebufferManager::createRenderPass(VkDevice device, const FramebufferConfig& config, VkRenderPass* renderPass) {
    std::array<VkAttachmentDescription, 2> attachments = {};

    // Color attachment
    attachments[0].format = config.colorFormat;
    attachments[0].samples = config.sampleCount;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Depth attachment
    attachments[1].format = config.depthFormat;
    attachments[1].samples = config.sampleCount;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies = {};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    return vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass);
}

VkResult FramebufferManager::createDepthStencil(VkDevice device, VkPhysicalDevice physicalDevice,
                                               const FramebufferConfig& config, DepthStencil* depthStencil) {
    VkResult result = createImage(device, physicalDevice,
                                 config.width, config.height,
                                 config.depthFormat,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 config.sampleCount,
                                 &depthStencil->image,
                                 &depthStencil->memory);

    if (result != VK_SUCCESS) {
        ocean::error("Failed to create depth stencil image");
        return result;
    }

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (hasStencilComponent(config.depthFormat)) {
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    result = createImageView(device, depthStencil->image, config.depthFormat,
                           aspectMask, &depthStencil->view);

    if (result != VK_SUCCESS) {
        ocean::error("Failed to create depth stencil image view");
        vkDestroyImage(device, depthStencil->image, nullptr);
        vkFreeMemory(device, depthStencil->memory, nullptr);
        return result;
    }

    return VK_SUCCESS;
}

void FramebufferManager::destroyDepthStencil(VkDevice device, DepthStencil* depthStencil) {
    if (depthStencil->view != VK_NULL_HANDLE) {
        vkDestroyImageView(device, depthStencil->view, nullptr);
        depthStencil->view = VK_NULL_HANDLE;
    }
    if (depthStencil->image != VK_NULL_HANDLE) {
        vkDestroyImage(device, depthStencil->image, nullptr);
        depthStencil->image = VK_NULL_HANDLE;
    }
    if (depthStencil->memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, depthStencil->memory, nullptr);
        depthStencil->memory = VK_NULL_HANDLE;
    }
}

VkResult FramebufferManager::createFramebuffers(VkDevice device, VkRenderPass renderPass,
                                               const std::vector<VkImageView>& swapChainImageViews,
                                               const DepthStencil& depthStencil,
                                               const FramebufferConfig& config,
                                               std::vector<VkFramebuffer>& framebuffers) {
    framebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthStencil.view
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = config.width;
        framebufferInfo.height = config.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);
        if (result != VK_SUCCESS) {
            ocean::error("Failed to create framebuffer " + std::to_string(i));

            // Clean up previously created framebuffers
            for (size_t j = 0; j < i; j++) {
                vkDestroyFramebuffer(device, framebuffers[j], nullptr);
            }
            framebuffers.clear();
            return result;
        }
    }

    return VK_SUCCESS;
}

void FramebufferManager::destroyFramebuffers(VkDevice device, std::vector<VkFramebuffer>& framebuffers) {
    for (auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    framebuffers.clear();
}

VkFormat FramebufferManager::findDepthFormat(VkPhysicalDevice physicalDevice) {
    return findSupportedFormat(physicalDevice,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat FramebufferManager::findSupportedFormat(VkPhysicalDevice physicalDevice,
                                                const std::vector<VkFormat>& candidates,
                                                VkImageTiling tiling,
                                                VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    ocean::error("Failed to find supported format");
    return VK_FORMAT_UNDEFINED;
}

bool FramebufferManager::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkResult FramebufferManager::createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                                        uint32_t width, uint32_t height, VkFormat format,
                                        VkImageTiling tiling, VkImageUsageFlags usage,
                                        VkMemoryPropertyFlags properties, VkSampleCountFlagBits sampleCount,
                                        VkImage* image, VkDeviceMemory* imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = sampleCount;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(device, &imageInfo, nullptr, image);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(device, &allocInfo, nullptr, imageMemory);
    if (result != VK_SUCCESS) {
        vkDestroyImage(device, *image, nullptr);
        return result;
    }

    vkBindImageMemory(device, *image, *imageMemory, 0);
    return VK_SUCCESS;
}

VkResult FramebufferManager::createImageView(VkDevice device, VkImage image, VkFormat format,
                                            VkImageAspectFlags aspectFlags, VkImageView* imageView) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    return vkCreateImageView(device, &viewInfo, nullptr, imageView);
}

uint32_t FramebufferManager::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                                           VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    ocean::error("Failed to find suitable memory type");
    return 0;
}

}} // namespace coregfx::platform