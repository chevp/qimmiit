#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace coregfx {
namespace platform {

class VulkanInitializer {
public:
    struct InstanceCreateInfo {
        std::string applicationName;
        std::string engineName;
        bool enableValidation = false;
        std::vector<const char*> requiredExtensions;
        std::vector<const char*> requiredLayers;
    };

    struct DeviceCreateInfo {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceFeatures enabledFeatures = {};
        std::vector<const char*> enabledExtensions;
        VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT;
    };

    // Instance creation utilities
    static VkResult createInstance(const InstanceCreateInfo& createInfo, VkInstance* instance);
    static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);
    static std::vector<const char*> getRequiredExtensions(bool enableValidation = false);

    // Physical device utilities
    static VkResult enumeratePhysicalDevices(VkInstance instance, std::vector<VkPhysicalDevice>& devices);
    static int32_t selectBestPhysicalDevice(const std::vector<VkPhysicalDevice>& devices);
    static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);

    // Debug utilities
    static VkResult setupDebugMessenger(VkInstance instance, VkDebugReportCallbackEXT* debugReportCallback);
    static void destroyDebugMessenger(VkInstance instance, VkDebugReportCallbackEXT debugReportCallback);

private:
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& extensions);
    static uint32_t rateDeviceSuitability(VkPhysicalDevice device);
};

}} // namespace coregfx::platform