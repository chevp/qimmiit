#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace coregfx {
namespace platform {

class GPUSelector {
public:
    struct DeviceInfo {
        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        uint32_t score;
        bool suitable;
    };

    GPUSelector() = default;
    ~GPUSelector() = default;

    // Primary GPU selection method (matches original getIndexOfBestGraphicalUnit)
    static int32_t getIndexOfBestGraphicalUnit(std::vector<VkPhysicalDevice>& devices);

    // Enhanced GPU selection with detailed scoring
    static int32_t selectBestPhysicalDevice(const std::vector<VkPhysicalDevice>& devices,
                                          VkSurfaceKHR surface = VK_NULL_HANDLE);

    // Device evaluation
    static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);
    static uint32_t rateDeviceSuitability(VkPhysicalDevice device);

    // Device information gathering
    static std::vector<DeviceInfo> evaluateDevices(const std::vector<VkPhysicalDevice>& devices,
                                                   VkSurfaceKHR surface = VK_NULL_HANDLE);

    // Device capability checks
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device,
                                           const std::vector<const char*>& extensions);
    static bool hasRequiredQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);

private:
    // Scoring helpers
    static uint32_t scoreDeviceType(VkPhysicalDeviceType deviceType);
    static uint32_t scoreMemorySize(uint64_t memorySize);
    static uint32_t scoreApiVersion(uint32_t apiVersion);

    // Required extensions for basic functionality
    static std::vector<const char*> getRequiredDeviceExtensions();
};

}} // namespace coregfx::platform