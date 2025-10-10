#include <coregfx/util/vulkan_initializer.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <set>
#include <chrono>

namespace coregfx {
namespace platform {

VkResult VulkanInitializer::createInstance(const InstanceCreateInfo& createInfo, VkInstance* instance) {
#ifndef NDEBUG
    ocean::info("=== VULKAN INSTANCE CREATION START ===");
    ocean::info("Application name: " + createInfo.applicationName);
    ocean::info("Validation requested: " + std::string(createInfo.enableValidation ? "true" : "false"));
#endif

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = createInfo.applicationName.c_str();
    appInfo.pEngineName = createInfo.engineName.c_str();
    appInfo.apiVersion = VK_API_VERSION_1_0;

#ifndef NDEBUG
    ocean::info("Vulkan API version: " + std::to_string(VK_VERSION_MAJOR(VK_API_VERSION_1_0)) + "." +
               std::to_string(VK_VERSION_MINOR(VK_API_VERSION_1_0)) + "." +
               std::to_string(VK_VERSION_PATCH(VK_API_VERSION_1_0)));
#endif

    // Get platform-specific extensions
    std::vector<const char*> instanceExtensions = getRequiredExtensions(createInfo.enableValidation);

    // Add any additional required extensions
    for (const char* ext : createInfo.requiredExtensions) {
        instanceExtensions.push_back(ext);
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
    instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    if (!instanceExtensions.empty()) {
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

#ifndef NDEBUG
        ocean::info("Total instance extensions: " + std::to_string(instanceExtensions.size()));
        for (size_t i = 0; i < instanceExtensions.size(); i++) {
            ocean::info("  Extension " + std::to_string(i) + ": " + std::string(instanceExtensions[i]));
        }
#endif
    }

    // Setup validation layers
    std::vector<const char*> validationLayers = createInfo.requiredLayers;
    if (createInfo.enableValidation) {
        if (!checkValidationLayerSupport(validationLayers)) {
            ocean::warn("Requested validation layers not available");
        }

        if (validationLayers.empty()) {
            validationLayers.push_back("VK_LAYER_KHRONOS_validation");
        }

        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

#ifndef NDEBUG
        ocean::info("Enabling " + std::to_string(validationLayers.size()) + " validation layers");
        for (const char* layer : validationLayers) {
            ocean::info("  Layer: " + std::string(layer));
        }
#endif
    }

#ifndef NDEBUG
    ocean::info("Creating Vulkan instance...");
#endif
    auto startTime = std::chrono::high_resolution_clock::now();
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, instance);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    if (result == VK_SUCCESS) {
#ifndef NDEBUG
        ocean::info("Vulkan instance created successfully in " + std::to_string(duration.count() / 1000.0) + " ms");
#endif
    } else {
        ocean::error("Failed to create Vulkan instance. VkResult: " + std::to_string(result));
    }

    return result;
}

bool VulkanInitializer::checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            ocean::warn("Validation layer not found: " + std::string(layerName));
            return false;
        }
    }

    return true;
}

std::vector<const char*> VulkanInitializer::getRequiredExtensions(bool enableValidation) {
    std::vector<const char*> extensions;

    // Base surface extension
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifndef NDEBUG
    ocean::info("Base extensions: VK_KHR_SURFACE_EXTENSION_NAME");
#endif

    // Platform-specific surface extensions
#if defined(_WIN32)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#ifndef NDEBUG
    ocean::info("Platform: Windows, adding VK_KHR_WIN32_SURFACE_EXTENSION_NAME");
#endif
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#ifndef NDEBUG
    ocean::info("Platform: Android, adding VK_KHR_ANDROID_SURFACE_EXTENSION_NAME");
#endif
#elif defined(_DIRECT2DISPLAY)
    extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#ifndef NDEBUG
    ocean::info("Platform: Direct2Display, adding VK_KHR_DISPLAY_EXTENSION_NAME");
#endif
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#ifndef NDEBUG
    ocean::info("Platform: Wayland, adding VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME");
#endif
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#ifndef NDEBUG
    ocean::info("Platform: XCB, adding VK_KHR_XCB_SURFACE_EXTENSION_NAME");
#endif
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#ifndef NDEBUG
    ocean::info("Platform: macOS, adding VK_MVK_MACOS_SURFACE_EXTENSION_NAME");
#endif
#endif

#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    if (enableValidation) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#ifndef NDEBUG
        ocean::info("Adding debug report extension for validation");
#endif
    }

    return extensions;
}

VkResult VulkanInitializer::enumeratePhysicalDevices(VkInstance instance, std::vector<VkPhysicalDevice>& devices) {
    uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (result != VK_SUCCESS || deviceCount == 0) {
        ocean::error("Failed to find GPUs with Vulkan support");
        return result;
    }

    devices.resize(deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

#ifndef NDEBUG
    ocean::info("Found " + std::to_string(deviceCount) + " physical devices");
#endif

    return result;
}

int32_t VulkanInitializer::selectBestPhysicalDevice(const std::vector<VkPhysicalDevice>& devices) {
    if (devices.empty()) {
        return -1;
    }

    int32_t bestDevice = 0;
    uint32_t highestScore = 0;

    for (size_t i = 0; i < devices.size(); i++) {
        uint32_t score = rateDeviceSuitability(devices[i]);

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

#ifndef NDEBUG
        ocean::info("Device " + std::to_string(i) + ": " + std::string(deviceProperties.deviceName) +
                   " (Score: " + std::to_string(score) + ")");
#endif

        if (score > highestScore) {
            highestScore = score;
            bestDevice = static_cast<int32_t>(i);
        }
    }

    return bestDevice;
}

bool VulkanInitializer::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Basic suitability check
    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
           deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

uint32_t VulkanInitializer::rateDeviceSuitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    uint32_t score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    return score;
}

VkResult VulkanInitializer::setupDebugMessenger(VkInstance instance, VkDebugReportCallbackEXT* debugReportCallback) {
    if (debugReportCallback == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Implementation would set up the debug messenger
    // This is a placeholder for the actual implementation
    return VK_SUCCESS;
}

void VulkanInitializer::destroyDebugMessenger(VkInstance instance, VkDebugReportCallbackEXT debugReportCallback) {
    // Implementation would clean up the debug messenger
}

bool VulkanInitializer::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& extensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

}} // namespace coregfx::platform