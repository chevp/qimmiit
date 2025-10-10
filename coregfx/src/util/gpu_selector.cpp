#include <coregfx/util/gpu_selector.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <algorithm>
#include <string>

namespace coregfx {
namespace platform {

int32_t GPUSelector::getIndexOfBestGraphicalUnit(std::vector<VkPhysicalDevice>& devices) {
#ifndef NDEBUG
    ocean::info("=== GPU SELECTION START ===");
    ocean::info("Available devices: " + std::to_string(devices.size()));
#endif

    if (devices.empty()) {
        ocean::error("No physical devices available");
        return -1;
    }

    int32_t bestIndex = -1;
    uint32_t bestScore = 0;
    bool foundDiscrete = false;

    for (size_t i = 0; i < devices.size(); ++i) {
        VkPhysicalDeviceProperties deviceProps;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProps);

#ifndef NDEBUG
        ocean::info("Device " + std::to_string(i) + ": " + std::string(deviceProps.deviceName));
        ocean::info("  Type: " + std::to_string(deviceProps.deviceType));
        ocean::info("  API Version: " + std::to_string(VK_VERSION_MAJOR(deviceProps.apiVersion)) + "." +
                   std::to_string(VK_VERSION_MINOR(deviceProps.apiVersion)) + "." +
                   std::to_string(VK_VERSION_PATCH(deviceProps.apiVersion)));
#endif

        uint32_t score = rateDeviceSuitability(devices[i]);
#ifndef NDEBUG
        ocean::info("  Suitability Score: " + std::to_string(score));
#endif

        bool isDiscrete = (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        bool isIntegrated = (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);

        // Priority logic: Prefer discrete GPUs, then best score
        bool shouldSelect = false;
        if (!foundDiscrete && isDiscrete) {
            // First discrete GPU found
            shouldSelect = true;
            foundDiscrete = true;
#ifndef NDEBUG
            ocean::info("  -> Selecting as first discrete GPU");
#endif
        } else if (foundDiscrete && isDiscrete && score > bestScore) {
            // Better discrete GPU
            shouldSelect = true;
#ifndef NDEBUG
            ocean::info("  -> Selecting as better discrete GPU");
#endif
        } else if (!foundDiscrete && isIntegrated && score > bestScore) {
            // Better integrated GPU (no discrete found yet)
            shouldSelect = true;
#ifndef NDEBUG
            ocean::info("  -> Selecting as better integrated GPU");
#endif
        } else if (!foundDiscrete && !isIntegrated && !isDiscrete && score > bestScore) {
            // Other GPU type, no discrete/integrated found yet
            shouldSelect = true;
#ifndef NDEBUG
            ocean::info("  -> Selecting as best available GPU");
#endif
        }

        if (shouldSelect) {
            bestIndex = static_cast<int32_t>(i);
            bestScore = score;
        }
    }

    if (bestIndex >= 0) {
        VkPhysicalDeviceProperties deviceProps;
        vkGetPhysicalDeviceProperties(devices[bestIndex], &deviceProps);
#ifndef NDEBUG
        ocean::info("Selected GPU " + std::to_string(bestIndex) + ": " + std::string(deviceProps.deviceName));
#endif
    } else {
        ocean::error("No suitable GPU found");
    }

#ifndef NDEBUG
    ocean::info("=== GPU SELECTION COMPLETE ===");
#endif
    return bestIndex;
}

int32_t GPUSelector::selectBestPhysicalDevice(const std::vector<VkPhysicalDevice>& devices, VkSurfaceKHR surface) {
    if (devices.empty()) {
        return -1;
    }

    std::vector<DeviceInfo> deviceInfos = evaluateDevices(devices, surface);

    // Sort by score (descending)
    std::sort(deviceInfos.begin(), deviceInfos.end(),
             [](const DeviceInfo& a, const DeviceInfo& b) {
                 if (a.suitable != b.suitable) {
                     return a.suitable > b.suitable; // Suitable devices first
                 }
                 return a.score > b.score;
             });

    // Return index of best suitable device
    for (size_t i = 0; i < deviceInfos.size(); ++i) {
        if (deviceInfos[i].suitable) {
            // Find original index
            for (size_t j = 0; j < devices.size(); ++j) {
                if (devices[j] == deviceInfos[i].device) {
                    return static_cast<int32_t>(j);
                }
            }
        }
    }

    return -1; // No suitable device found
}

bool GPUSelector::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Check required extensions
    if (!checkDeviceExtensionSupport(device, getRequiredDeviceExtensions())) {
        return false;
    }

    // Check queue families
    if (!hasRequiredQueueFamilies(device, surface)) {
        return false;
    }

    // Additional checks can be added here
    return true;
}

uint32_t GPUSelector::rateDeviceSuitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    uint32_t score = 0;

    // Device type scoring
    score += scoreDeviceType(deviceProperties.deviceType);

    // API version scoring
    score += scoreApiVersion(deviceProperties.apiVersion);

    // Memory size scoring
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

    uint64_t totalMemory = 0;
    for (uint32_t i = 0; i < memProperties.memoryHeapCount; ++i) {
        if (memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            totalMemory += memProperties.memoryHeaps[i].size;
        }
    }
    score += scoreMemorySize(totalMemory);

    // Feature scoring
    if (deviceFeatures.geometryShader) score += 100;
    if (deviceFeatures.tessellationShader) score += 50;
    if (deviceFeatures.multiViewport) score += 25;
    if (deviceFeatures.samplerAnisotropy) score += 25;

    return score;
}

std::vector<GPUSelector::DeviceInfo> GPUSelector::evaluateDevices(const std::vector<VkPhysicalDevice>& devices,
                                                                 VkSurfaceKHR surface) {
    std::vector<DeviceInfo> deviceInfos;
    deviceInfos.reserve(devices.size());

    for (const auto& device : devices) {
        DeviceInfo info;
        info.device = device;
        vkGetPhysicalDeviceProperties(device, &info.properties);
        vkGetPhysicalDeviceFeatures(device, &info.features);
        info.score = rateDeviceSuitability(device);
        info.suitable = isDeviceSuitable(device, surface);

        deviceInfos.push_back(info);
    }

    return deviceInfos;
}

bool GPUSelector::checkDeviceExtensionSupport(VkPhysicalDevice device,
                                             const std::vector<const char*>& extensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    for (const char* requiredExtension : extensions) {
        bool found = false;
        for (const auto& extension : availableExtensions) {
            if (strcmp(requiredExtension, extension.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

bool GPUSelector::hasRequiredQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    bool hasGraphics = false;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            hasGraphics = true;
            break;
        }
    }

    return hasGraphics;
}

uint32_t GPUSelector::scoreDeviceType(VkPhysicalDeviceType deviceType) {
    switch (deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return 1000;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return 500;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return 100;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return 50;
        default:
            return 0;
    }
}

uint32_t GPUSelector::scoreMemorySize(uint64_t memorySize) {
    // Score based on GB of memory (capped at reasonable value)
    uint32_t memoryGB = static_cast<uint32_t>(memorySize / (1024 * 1024 * 1024));
    return std::min(memoryGB * 10, 500u); // Cap at 50GB equivalent
}

uint32_t GPUSelector::scoreApiVersion(uint32_t apiVersion) {
    uint32_t major = VK_VERSION_MAJOR(apiVersion);
    uint32_t minor = VK_VERSION_MINOR(apiVersion);

    // Score based on API version
    return (major * 100) + (minor * 10);
}

std::vector<const char*> GPUSelector::getRequiredDeviceExtensions() {
    return {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

}} // namespace coregfx::platform