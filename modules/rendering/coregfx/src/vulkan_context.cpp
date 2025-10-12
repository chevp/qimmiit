#include <coregfx/vulkan_context.hpp>
#include <coregfx/util/vulkan_state_logger.hpp>
#include <coregfx/core/CryoErrorCodes.hpp>
#include <coregfx/core/ocean_log.hpp>

// Helper function to check validation layer support
bool VulkanContext::checkValidationLayerSupport() {
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
            return false;
        }
    }

    return true;
}

// Vulkan setup functions
void VulkanContext::createInstance() {
    const std::vector<const char*> instanceExtensions = { "VK_EXT_debug_utils" };

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        // Set error code for crash diagnosis
        cryo::setCryoErrorCode(cgfx::CRYO_ERROR_VULKAN_INSTANCE_FAILED);

        // Throw with detailed VkResult - caller should handle logging
        throw std::runtime_error(
            "Failed to create Vulkan instance! VkResult: " +
            std::to_string(result) + " (" +
            std::string(cryo::vkResultToString(result)) + ")"
        );
    }

    // Log instance state after creation
    auto& logger = coregfx::util::VulkanStateLogger::getInstance();
    logger.logInstanceState(instance, coregfx::util::LogCategory::INITIALIZATION);
}

void VulkanContext::setupDebugMessenger() {
    if (!enableValidationLayers) return;
/*
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    // Debug messenger creation logic

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, &createInfo, pAllocator, pDebugMessenger);
    }
    else {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
    if (vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }*/
}

void VulkanContext::createSurface() {
    // Surface creation logic (platform-dependent)
    // Example for GLFW:
    // if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
    //     throw std::runtime_error("Failed to create window surface!");
    // }
}

void VulkanContext::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    // Log physical device state after selection
    auto& logger = coregfx::util::VulkanStateLogger::getInstance();
    logger.logPhysicalDeviceState(physicalDevice, coregfx::util::LogCategory::DEVICE_STATE);
}

void VulkanContext::createLogicalDevice() {
    VkDeviceCreateInfo createInfo{};
    // Logical device creation logic

    VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
        // Set error code for crash diagnosis
        cryo::setCryoErrorCode(cgfx::CRYO_ERROR_VULKAN_DEVICE_FAILED);

        // Throw with detailed VkResult - caller should handle logging
        throw std::runtime_error(
            "Failed to create logical device! VkResult: " +
            std::to_string(result) + " (" +
            std::string(cryo::vkResultToString(result)) + ")"
        );
    }

    vkGetDeviceQueue(device, 0, 0, &graphicsQueue);
    vkGetDeviceQueue(device, 0, 0, &presentQueue);

    // Log device state after creation
    auto& logger = coregfx::util::VulkanStateLogger::getInstance();
    logger.logDeviceState(device, coregfx::util::LogCategory::DEVICE_STATE);
}

void VulkanContext::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) {
        // Set error code for crash diagnosis
        cryo::setCryoErrorCode(cgfx::CRYO_ERROR_VULKAN_COMMAND_BUFFER_FAILED);

        // Throw with detailed VkResult - caller should handle logging
        throw std::runtime_error(
            "Failed to create command pool! VkResult: " +
            std::to_string(result) + " (" +
            std::string(cryo::vkResultToString(result)) + ")"
        );
    }

    // Log command pool state after creation
    auto& logger = coregfx::util::VulkanStateLogger::getInstance();
    logger.logCommandPoolState(device, commandPool, coregfx::util::LogCategory::COMMAND_BUFFERS);
}

// Function to destroy Debug Messenger
void VulkanContext::destroyDebugMessenger(const VkAllocationCallbacks* pAllocator) {
    // Load the function pointer for vkDestroyDebugUtilsMessengerEXT
    /*auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
    else {
        std::cerr << "Failed to load vkDestroyDebugUtilsMessengerEXT." << std::endl;
    }*/
}
