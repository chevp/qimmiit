/**
 * Copyright (C) by chevp
 * Vulkan State Logger Implementation
 */

#include <coregfx/util/vulkan_state_logger.hpp>
#include <coregfx/util/compact_logger.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <string.h>

namespace coregfx {
namespace util {

bool VulkanStateLogger::initialize(const LogSettings& settings) {
    std::lock_guard<std::mutex> lock(m_logMutex);

    m_settings = settings;
    m_startTime = std::chrono::steady_clock::now();

    if (m_settings.level == LogLevel::NONE) {
        m_initialized = true;
        return true;
    }

    // Initialize compact logger if needed
    if (m_settings.format == OutputFormat::COMPACT || m_settings.format == OutputFormat::BOTH) {
        CompactLogger::getInstance().initialize(m_settings.compactFile, m_settings.consoleOutput);
    }

    // DISABLED: Do not create vulkan_state.log file
    // Vulkan state logging is disabled to reduce disk I/O
    // Use compact logging format instead if needed
    /*
    if (m_settings.format == OutputFormat::VERBOSE || m_settings.format == OutputFormat::BOTH) {
        auto mode = m_settings.appendMode ? std::ios::app : std::ios::trunc;
        m_logFile = std::make_unique<std::ofstream>(m_settings.outputFile, mode);

        if (!m_logFile->is_open()) {
            std::cerr << "Failed to open Vulkan state log file: " << m_settings.outputFile << std::endl;
            return false;
        }
    }
    */

    // Write header for verbose format
    if (m_logFile) {
        *m_logFile << "===============================================" << std::endl;
        *m_logFile << "Vulkan State Logger Initialized" << std::endl;
        *m_logFile << "Timestamp: " << formatTimestamp() << std::endl;
        *m_logFile << "Log Level: " << static_cast<int>(m_settings.level) << std::endl;
        *m_logFile << "Categories: 0x" << std::hex << m_settings.categories << std::dec << std::endl;
        *m_logFile << "===============================================" << std::endl;
        *m_logFile << std::endl;
    }

    m_initialized = true;
    return true;
}

LogSettings VulkanStateLogger::parseCommandLineArgs(int argc, char* argv[]) {
    LogSettings settings;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--vulkan-log-level" && i + 1 < argc) {
            int level = std::atoi(argv[++i]);
            settings.level = static_cast<LogLevel>(std::clamp(level, 0, 3));
        }
        else if (arg == "--vulkan-log-categories" && i + 1 < argc) {
            settings.categories = std::strtoul(argv[++i], nullptr, 0);
        }
        else if (arg == "--vulkan-log-file" && i + 1 < argc) {
            settings.outputFile = argv[++i];
        }
        else if (arg == "--vulkan-log-console") {
            settings.consoleOutput = true;
        }
        else if (arg == "--vulkan-log-append") {
            settings.appendMode = true;
        }
        else if (arg == "--vulkan-log-init") {
            settings.categories |= static_cast<uint32_t>(LogCategory::INITIALIZATION);
        }
        else if (arg == "--vulkan-log-device") {
            settings.categories |= static_cast<uint32_t>(LogCategory::DEVICE_STATE);
        }
        else if (arg == "--vulkan-log-memory") {
            settings.categories |= static_cast<uint32_t>(LogCategory::MEMORY_STATE);
        }
        else if (arg == "--vulkan-log-commands") {
            settings.categories |= static_cast<uint32_t>(LogCategory::COMMAND_BUFFERS);
        }
        else if (arg == "--vulkan-log-pipelines") {
            settings.categories |= static_cast<uint32_t>(LogCategory::PIPELINES);
        }
        else if (arg == "--vulkan-log-renderpasses") {
            settings.categories |= static_cast<uint32_t>(LogCategory::RENDER_PASSES);
        }
        else if (arg == "--vulkan-log-descriptors") {
            settings.categories |= static_cast<uint32_t>(LogCategory::DESCRIPTORS);
        }
        else if (arg == "--vulkan-log-sync") {
            settings.categories |= static_cast<uint32_t>(LogCategory::SYNCHRONIZATION);
        }
        else if (arg == "--vulkan-log-swapchain") {
            settings.categories |= static_cast<uint32_t>(LogCategory::SWAPCHAIN);
        }
        else if (arg == "--vulkan-log-buffers") {
            settings.categories |= static_cast<uint32_t>(LogCategory::BUFFERS);
        }
        else if (arg == "--vulkan-log-textures") {
            settings.categories |= static_cast<uint32_t>(LogCategory::TEXTURES);
        }
        else if (arg == "--vulkan-log-shaders") {
            settings.categories |= static_cast<uint32_t>(LogCategory::SHADERS);
        }
        else if (arg == "--vulkan-log-gameloop") {
            settings.categories |= static_cast<uint32_t>(LogCategory::GAME_LOOP);
        }
        else if (arg == "--vulkan-log-all") {
            settings.categories = static_cast<uint32_t>(LogCategory::ALL);
        }
        else if (arg == "--vulkan-log-format" && i + 1 < argc) {
            std::string format = argv[++i];
            if (format == "compact") {
                settings.format = OutputFormat::COMPACT;
            } else if (format == "both") {
                settings.format = OutputFormat::BOTH;
            } else {
                settings.format = OutputFormat::VERBOSE;
            }
        }
        else if (arg == "--vulkan-compact-file" && i + 1 < argc) {
            settings.compactFile = argv[++i];
        }
    }

    return settings;
}

LogSettings VulkanStateLogger::parseCommandLineString(const std::string& cmdLine) {
    LogSettings settings;

    if (cmdLine.empty()) {
        return settings;
    }

    // Simple tokenization by spaces (could be improved for quoted arguments)
    std::vector<std::string> tokens;
    std::istringstream iss(cmdLine);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    // Convert to argc/argv format
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>("program")); // dummy program name
    for (auto& t : tokens) {
        argv.push_back(const_cast<char*>(t.c_str()));
    }

    return parseCommandLineArgs(static_cast<int>(argv.size()), argv.data());
}

void VulkanStateLogger::beginFrame(uint32_t frameNumber) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_currentFrame = frameNumber;

    if (shouldLog(LogCategory::GAME_LOOP)) {
        writeToLog("=== FRAME " + std::to_string(frameNumber) + " BEGIN ===", LogCategory::GAME_LOOP);
    }
}

void VulkanStateLogger::endFrame() {
    std::lock_guard<std::mutex> lock(m_logMutex);

    if (shouldLog(LogCategory::GAME_LOOP)) {
        writeToLog("=== FRAME " + std::to_string(m_currentFrame) + " END ===", LogCategory::GAME_LOOP);
    }
}

void VulkanStateLogger::setGameLoopPhase(const std::string& phase) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_currentPhase = phase;

    if (shouldLog(LogCategory::GAME_LOOP)) {
        writeToLog("Game Loop Phase: " + phase, LogCategory::GAME_LOOP);
    }
}

void VulkanStateLogger::logInstanceState(VkInstance instance, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractInstanceInfo(instance);

    // Verbose format
    if (m_settings.format == OutputFormat::VERBOSE || m_settings.format == OutputFormat::BOTH) {
        writeToLog("INSTANCE STATE:\n" + info, category);
    }

    // Compact format
    if (m_settings.format == OutputFormat::COMPACT || m_settings.format == OutputFormat::BOTH) {
        // Extract basic info for compact logging
        uint32_t apiVersion = 0;
        if (vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS) {
            uint32_t layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

            CompactLogger::getInstance().logVulkanInstance(instance, apiVersion, layerCount, extensionCount);
        }
    }
}

void VulkanStateLogger::logPhysicalDeviceState(VkPhysicalDevice physicalDevice, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractPhysicalDeviceInfo(physicalDevice);
    writeToLog("PHYSICAL DEVICE STATE:\n" + info, category);
}

void VulkanStateLogger::logDeviceState(VkDevice device, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractDeviceInfo(device);
    writeToLog("DEVICE STATE:\n" + info, category);
}

void VulkanStateLogger::logMemoryState(VkDevice device, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractMemoryInfo(device);
    writeToLog("MEMORY STATE:\n" + info, category);
}

void VulkanStateLogger::logCommandPoolState(VkDevice device, VkCommandPool commandPool, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractCommandPoolInfo(device, commandPool);
    writeToLog("COMMAND POOL STATE:\n" + info, category);
}

void VulkanStateLogger::logCommandBufferState(VkCommandBuffer commandBuffer, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractCommandBufferInfo(commandBuffer);
    writeToLog("COMMAND BUFFER STATE:\n" + info, category);
}

void VulkanStateLogger::logRenderPassState(VkDevice device, VkRenderPass renderPass, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractRenderPassInfo(device, renderPass);
    writeToLog("RENDER PASS STATE:\n" + info, category);
}

void VulkanStateLogger::logPipelineState(VkDevice device, VkPipeline pipeline, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractPipelineInfo(device, pipeline);
    writeToLog("PIPELINE STATE:\n" + info, category);
}

void VulkanStateLogger::logDescriptorSetState(VkDevice device, VkDescriptorSet descriptorSet, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractDescriptorSetInfo(device, descriptorSet);
    writeToLog("DESCRIPTOR SET STATE:\n" + info, category);
}

void VulkanStateLogger::logSwapchainState(VkDevice device, VkSwapchainKHR swapchain, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractSwapchainInfo(device, swapchain);
    writeToLog("SWAPCHAIN STATE:\n" + info, category);
}

void VulkanStateLogger::logBufferState(VkDevice device, VkBuffer buffer, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractBufferInfo(device, buffer);
    writeToLog("BUFFER STATE:\n" + info, category);
}

void VulkanStateLogger::logImageState(VkDevice device, VkImage image, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractImageInfo(device, image);
    writeToLog("IMAGE STATE:\n" + info, category);
}

void VulkanStateLogger::logSemaphoreState(VkDevice device, VkSemaphore semaphore, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractSemaphoreInfo(device, semaphore);
    writeToLog("SEMAPHORE STATE:\n" + info, category);
}

void VulkanStateLogger::logFenceState(VkDevice device, VkFence fence, LogCategory category) {
    if (!shouldLog(category)) return;

    std::string info = extractFenceInfo(device, fence);
    writeToLog("FENCE STATE:\n" + info, category);
}

void VulkanStateLogger::logEvent(const std::string& event, LogCategory category) {
    if (!shouldLog(category)) return;

    writeToLog("EVENT: " + event, category);
}

void VulkanStateLogger::logError(const std::string& error, LogCategory category) {
    writeToLog("ERROR: " + error, category);
}

void VulkanStateLogger::logWarning(const std::string& warning, LogCategory category) {
    writeToLog("WARNING: " + warning, category);
}

void VulkanStateLogger::captureCompleteState(VkInstance instance, VkDevice device, const std::string& snapshotName) {
    if (!m_initialized || m_settings.level == LogLevel::NONE) return;

    std::lock_guard<std::mutex> lock(m_logMutex);

    std::string name = snapshotName.empty() ? ("Snapshot_Frame_" + std::to_string(m_currentFrame)) : snapshotName;

    writeToLog("=== COMPLETE STATE SNAPSHOT: " + name + " ===", LogCategory::ALL);

    if (instance != VK_NULL_HANDLE) {
        logInstanceState(instance, LogCategory::INITIALIZATION);
    }

    if (device != VK_NULL_HANDLE) {
        logDeviceState(device, LogCategory::DEVICE_STATE);
        logMemoryState(device, LogCategory::MEMORY_STATE);
    }

    writeToLog("=== END SNAPSHOT: " + name + " ===", LogCategory::ALL);
}

void VulkanStateLogger::shutdown() {
    std::lock_guard<std::mutex> lock(m_logMutex);

    // Shutdown compact logger
    if (m_settings.format == OutputFormat::COMPACT || m_settings.format == OutputFormat::BOTH) {
        CompactLogger::getInstance().shutdown();
    }

    // Shutdown verbose logger
    if (m_logFile && m_logFile->is_open()) {
        *m_logFile << "===============================================" << std::endl;
        *m_logFile << "Vulkan State Logger Shutdown" << std::endl;
        *m_logFile << "Timestamp: " << formatTimestamp() << std::endl;
        *m_logFile << "Total Frames Logged: " << m_currentFrame << std::endl;
        *m_logFile << "===============================================" << std::endl;
        m_logFile->close();
    }

    m_initialized = false;
}

void VulkanStateLogger::writeToLog(const std::string& message, LogCategory category) {
    if (!m_initialized || m_settings.level == LogLevel::NONE) return;

    std::ostringstream oss;

    if (m_settings.timestampEnabled) {
        oss << "[" << formatTimestamp() << "] ";
    }

    if (m_settings.frameNumberEnabled && m_currentFrame > 0) {
        oss << "[Frame:" << m_currentFrame << "] ";
    }

    if (!m_currentPhase.empty()) {
        oss << "[" << m_currentPhase << "] ";
    }

    oss << "[" << categoryToString(category) << "] ";
    oss << message << std::endl;

    std::string logLine = oss.str();

    if (m_logFile && m_logFile->is_open()) {
        *m_logFile << logLine;
        m_logFile->flush();
    }

    if (m_settings.consoleOutput) {
        std::cout << logLine;
    }
}

std::string VulkanStateLogger::formatTimestamp() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime);

    auto ms = elapsed.count();
    auto seconds = ms / 1000;
    auto minutes = seconds / 60;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << (minutes % 60)
        << ":" << std::setw(2) << (seconds % 60)
        << "." << std::setw(3) << (ms % 1000);

    return oss.str();
}

std::string VulkanStateLogger::formatFrameInfo() const {
    return "Frame:" + std::to_string(m_currentFrame);
}

std::string VulkanStateLogger::categoryToString(LogCategory category) const {
    switch (category) {
        case LogCategory::INITIALIZATION: return "INIT";
        case LogCategory::DEVICE_STATE: return "DEVICE";
        case LogCategory::MEMORY_STATE: return "MEMORY";
        case LogCategory::COMMAND_BUFFERS: return "COMMANDS";
        case LogCategory::PIPELINES: return "PIPELINES";
        case LogCategory::RENDER_PASSES: return "RENDERPASSES";
        case LogCategory::DESCRIPTORS: return "DESCRIPTORS";
        case LogCategory::SYNCHRONIZATION: return "SYNC";
        case LogCategory::SWAPCHAIN: return "SWAPCHAIN";
        case LogCategory::BUFFERS: return "BUFFERS";
        case LogCategory::TEXTURES: return "TEXTURES";
        case LogCategory::SHADERS: return "SHADERS";
        case LogCategory::GAME_LOOP: return "GAMELOOP";
        default: return "UNKNOWN";
    }
}

bool VulkanStateLogger::shouldLog(LogCategory category) const {
    return m_initialized &&
           m_settings.level != LogLevel::NONE &&
           (m_settings.categories & static_cast<uint32_t>(category)) != 0;
}

// Vulkan state extraction implementations
std::string VulkanStateLogger::extractInstanceInfo(VkInstance instance) {
    std::ostringstream oss;

    oss << "  Instance Handle: " << instance << std::endl;

    if (instance != VK_NULL_HANDLE) {
        // Get instance version
        uint32_t apiVersion = 0;
        if (vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS) {
            oss << "  API Version: " << VK_VERSION_MAJOR(apiVersion)
                << "." << VK_VERSION_MINOR(apiVersion)
                << "." << VK_VERSION_PATCH(apiVersion) << std::endl;
        }

        // Get available layers
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        if (layerCount > 0) {
            std::vector<VkLayerProperties> layers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

            oss << "  Available Layers (" << layerCount << "):" << std::endl;
            for (const auto& layer : layers) {
                oss << "    - " << layer.layerName << " (v" << layer.specVersion << ")" << std::endl;
            }
        }

        // Get available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        if (extensionCount > 0) {
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

            oss << "  Available Extensions (" << extensionCount << "):" << std::endl;
            for (const auto& ext : extensions) {
                oss << "    - " << ext.extensionName << " (v" << ext.specVersion << ")" << std::endl;
            }
        }
    }

    return oss.str();
}

std::string VulkanStateLogger::extractPhysicalDeviceInfo(VkPhysicalDevice physicalDevice) {
    std::ostringstream oss;

    oss << "  Physical Device Handle: " << physicalDevice << std::endl;

    if (physicalDevice != VK_NULL_HANDLE) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        oss << "  Device Name: " << properties.deviceName << std::endl;
        oss << "  Device Type: " << properties.deviceType << std::endl;
        oss << "  Vendor ID: 0x" << std::hex << properties.vendorID << std::dec << std::endl;
        oss << "  Device ID: 0x" << std::hex << properties.deviceID << std::dec << std::endl;
        oss << "  Driver Version: " << properties.driverVersion << std::endl;
        oss << "  API Version: " << VK_VERSION_MAJOR(properties.apiVersion)
            << "." << VK_VERSION_MINOR(properties.apiVersion)
            << "." << VK_VERSION_PATCH(properties.apiVersion) << std::endl;

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physicalDevice, &features);

        oss << "  Key Features:" << std::endl;
        oss << "    Geometry Shader: " << (features.geometryShader ? "Yes" : "No") << std::endl;
        oss << "    Tessellation Shader: " << (features.tessellationShader ? "Yes" : "No") << std::endl;
        oss << "    Multi Viewport: " << (features.multiViewport ? "Yes" : "No") << std::endl;
        oss << "    Anisotropy: " << (features.samplerAnisotropy ? "Yes" : "No") << std::endl;

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

        oss << "  Memory Information:" << std::endl;
        oss << "    Memory Types: " << memProps.memoryTypeCount << std::endl;
        oss << "    Memory Heaps: " << memProps.memoryHeapCount << std::endl;

        for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
            oss << "    Heap " << i << ": " << (memProps.memoryHeaps[i].size / (1024 * 1024))
                << " MB, Flags: 0x" << std::hex << memProps.memoryHeaps[i].flags << std::dec << std::endl;
        }
    }

    return oss.str();
}

std::string VulkanStateLogger::extractDeviceInfo(VkDevice device) {
    std::ostringstream oss;

    oss << "  Device Handle: " << device << std::endl;

    if (device != VK_NULL_HANDLE) {
        oss << "  Device Status: Active" << std::endl;

        // Note: Getting detailed device state would require tracking during creation
        // Since we don't have access to the creation info, we log what we can
    }

    return oss.str();
}

std::string VulkanStateLogger::extractMemoryInfo(VkDevice device) {
    std::ostringstream oss;

    oss << "  Device Handle: " << device << std::endl;

    if (device != VK_NULL_HANDLE) {
        oss << "  Memory State: Active" << std::endl;

        // Note: Detailed memory usage would require tracking allocations
        // This is a placeholder for basic memory state information
    }

    return oss.str();
}

std::string VulkanStateLogger::extractCommandPoolInfo(VkDevice device, VkCommandPool commandPool) {
    std::ostringstream oss;

    oss << "  Command Pool Handle: " << commandPool << std::endl;
    oss << "  Device: " << device << std::endl;

    if (commandPool != VK_NULL_HANDLE) {
        oss << "  Status: Active" << std::endl;
    }

    return oss.str();
}

std::string VulkanStateLogger::extractCommandBufferInfo(VkCommandBuffer commandBuffer) {
    std::ostringstream oss;

    oss << "  Command Buffer Handle: " << commandBuffer << std::endl;

    if (commandBuffer != VK_NULL_HANDLE) {
        oss << "  Status: Active" << std::endl;
    }

    return oss.str();
}

std::string VulkanStateLogger::extractRenderPassInfo(VkDevice device, VkRenderPass renderPass) {
    std::ostringstream oss;

    oss << "  Render Pass Handle: " << renderPass << std::endl;
    oss << "  Device: " << device << std::endl;

    return oss.str();
}

std::string VulkanStateLogger::extractPipelineInfo(VkDevice device, VkPipeline pipeline) {
    std::ostringstream oss;

    oss << "  Pipeline Handle: " << pipeline << std::endl;
    oss << "  Device: " << device << std::endl;

    return oss.str();
}

std::string VulkanStateLogger::extractDescriptorSetInfo(VkDevice device, VkDescriptorSet descriptorSet) {
    std::ostringstream oss;

    oss << "  Descriptor Set Handle: " << descriptorSet << std::endl;
    oss << "  Device: " << device << std::endl;

    return oss.str();
}

std::string VulkanStateLogger::extractSwapchainInfo(VkDevice device, VkSwapchainKHR swapchain) {
    std::ostringstream oss;

    oss << "  Swapchain Handle: " << swapchain << std::endl;
    oss << "  Device: " << device << std::endl;

    return oss.str();
}

std::string VulkanStateLogger::extractBufferInfo(VkDevice device, VkBuffer buffer) {
    std::ostringstream oss;

    oss << "  Buffer Handle: " << buffer << std::endl;
    oss << "  Device: " << device << std::endl;

    if (buffer != VK_NULL_HANDLE) {
        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device, buffer, &memReqs);

        oss << "  Memory Requirements:" << std::endl;
        oss << "    Size: " << memReqs.size << " bytes" << std::endl;
        oss << "    Alignment: " << memReqs.alignment << std::endl;
        oss << "    Memory Type Bits: 0x" << std::hex << memReqs.memoryTypeBits << std::dec << std::endl;
    }

    return oss.str();
}

std::string VulkanStateLogger::extractImageInfo(VkDevice device, VkImage image) {
    std::ostringstream oss;

    oss << "  Image Handle: " << image << std::endl;
    oss << "  Device: " << device << std::endl;

    if (image != VK_NULL_HANDLE) {
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, image, &memReqs);

        oss << "  Memory Requirements:" << std::endl;
        oss << "    Size: " << memReqs.size << " bytes" << std::endl;
        oss << "    Alignment: " << memReqs.alignment << std::endl;
        oss << "    Memory Type Bits: 0x" << std::hex << memReqs.memoryTypeBits << std::dec << std::endl;
    }

    return oss.str();
}

std::string VulkanStateLogger::extractSemaphoreInfo(VkDevice device, VkSemaphore semaphore) {
    std::ostringstream oss;

    oss << "  Semaphore Handle: " << semaphore << std::endl;
    oss << "  Device: " << device << std::endl;

    return oss.str();
}

std::string VulkanStateLogger::extractFenceInfo(VkDevice device, VkFence fence) {
    std::ostringstream oss;

    oss << "  Fence Handle: " << fence << std::endl;
    oss << "  Device: " << device << std::endl;

    if (fence != VK_NULL_HANDLE) {
        VkResult result = vkGetFenceStatus(device, fence);
        oss << "  Status: ";
        switch (result) {
            case VK_SUCCESS: oss << "Signaled"; break;
            case VK_NOT_READY: oss << "Unsignaled"; break;
            default: oss << "Error (" << result << ")"; break;
        }
        oss << std::endl;
    }

    return oss.str();
}

} // namespace util
} // namespace coregfx