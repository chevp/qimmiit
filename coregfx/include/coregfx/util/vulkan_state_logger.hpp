/**
 * Copyright (C) by chevp
 * Vulkan State Logger - Comprehensive state capture and logging system
 */

#pragma once

#ifndef VULKAN_STATE_LOGGER_HPP_
#define VULKAN_STATE_LOGGER_HPP_

#include <vulkan/vulkan.h>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <mutex>

namespace coregfx {
namespace util {

enum class LogLevel {
    NONE = 0,
    MINIMAL = 1,
    DETAILED = 2,
    VERBOSE = 3
};

enum class LogCategory : uint32_t {
    INITIALIZATION = 1U << 0,
    DEVICE_STATE = 1U << 1,
    MEMORY_STATE = 1U << 2,
    COMMAND_BUFFERS = 1U << 3,
    PIPELINES = 1U << 4,
    RENDER_PASSES = 1U << 5,
    DESCRIPTORS = 1U << 6,
    SYNCHRONIZATION = 1U << 7,
    SWAPCHAIN = 1U << 8,
    BUFFERS = 1U << 9,
    TEXTURES = 1U << 10,
    SHADERS = 1U << 11,
    GAME_LOOP = 1U << 12,
    ALL = 0xFFFFFFFFU
};

enum class OutputFormat {
    VERBOSE = 0,    // Original detailed format
    COMPACT = 1,    // SQL-style compact format
    BOTH = 2        // Both formats
};

struct LogSettings {
    LogLevel level = LogLevel::NONE;
    uint32_t categories = 0;
    std::string outputFile = "vulkan_state.log";
    bool timestampEnabled = true;
    bool frameNumberEnabled = true;
    bool consoleOutput = false;
    bool appendMode = false;
    OutputFormat format = OutputFormat::VERBOSE;
    std::string compactFile = "vulkan_compact.sql";
};

class VulkanStateLogger {
public:
    static VulkanStateLogger& getInstance() {
        static VulkanStateLogger instance;
        return instance;
    }

    // Delete copy constructor and assignment
    VulkanStateLogger(const VulkanStateLogger&) = delete;
    VulkanStateLogger& operator=(const VulkanStateLogger&) = delete;

    // Initialize logger with settings
    bool initialize(const LogSettings& settings);

    // Parse command line arguments for logging configuration
    static LogSettings parseCommandLineArgs(int argc, char* argv[]);
    static LogSettings parseCommandLineString(const std::string& cmdLine);

    // Control logging state
    void beginFrame(uint32_t frameNumber);
    void endFrame();
    void setGameLoopPhase(const std::string& phase);

    // Vulkan state logging methods
    void logInstanceState(VkInstance instance, LogCategory category = LogCategory::INITIALIZATION);
    void logPhysicalDeviceState(VkPhysicalDevice physicalDevice, LogCategory category = LogCategory::DEVICE_STATE);
    void logDeviceState(VkDevice device, LogCategory category = LogCategory::DEVICE_STATE);
    void logMemoryState(VkDevice device, LogCategory category = LogCategory::MEMORY_STATE);
    void logCommandPoolState(VkDevice device, VkCommandPool commandPool, LogCategory category = LogCategory::COMMAND_BUFFERS);
    void logCommandBufferState(VkCommandBuffer commandBuffer, LogCategory category = LogCategory::COMMAND_BUFFERS);
    void logRenderPassState(VkDevice device, VkRenderPass renderPass, LogCategory category = LogCategory::RENDER_PASSES);
    void logPipelineState(VkDevice device, VkPipeline pipeline, LogCategory category = LogCategory::PIPELINES);
    void logDescriptorSetState(VkDevice device, VkDescriptorSet descriptorSet, LogCategory category = LogCategory::DESCRIPTORS);
    void logSwapchainState(VkDevice device, VkSwapchainKHR swapchain, LogCategory category = LogCategory::SWAPCHAIN);
    void logBufferState(VkDevice device, VkBuffer buffer, LogCategory category = LogCategory::BUFFERS);
    void logImageState(VkDevice device, VkImage image, LogCategory category = LogCategory::TEXTURES);
    void logSemaphoreState(VkDevice device, VkSemaphore semaphore, LogCategory category = LogCategory::SYNCHRONIZATION);
    void logFenceState(VkDevice device, VkFence fence, LogCategory category = LogCategory::SYNCHRONIZATION);

    // Custom logging for specific events
    void logEvent(const std::string& event, LogCategory category = LogCategory::GAME_LOOP);
    void logError(const std::string& error, LogCategory category = LogCategory::ALL);
    void logWarning(const std::string& warning, LogCategory category = LogCategory::ALL);

    // Complete state snapshot
    void captureCompleteState(VkInstance instance, VkDevice device, const std::string& snapshotName = "");

    // Cleanup
    void shutdown();

private:
    VulkanStateLogger() = default;
    ~VulkanStateLogger() { shutdown(); }

    // Internal logging methods
    void writeToLog(const std::string& message, LogCategory category);
    std::string formatTimestamp() const;
    std::string formatFrameInfo() const;
    std::string categoryToString(LogCategory category) const;
    bool shouldLog(LogCategory category) const;

    // Vulkan state extraction helpers
    std::string extractInstanceInfo(VkInstance instance);
    std::string extractPhysicalDeviceInfo(VkPhysicalDevice physicalDevice);
    std::string extractDeviceInfo(VkDevice device);
    std::string extractMemoryInfo(VkDevice device);
    std::string extractCommandPoolInfo(VkDevice device, VkCommandPool commandPool);
    std::string extractCommandBufferInfo(VkCommandBuffer commandBuffer);
    std::string extractRenderPassInfo(VkDevice device, VkRenderPass renderPass);
    std::string extractPipelineInfo(VkDevice device, VkPipeline pipeline);
    std::string extractDescriptorSetInfo(VkDevice device, VkDescriptorSet descriptorSet);
    std::string extractSwapchainInfo(VkDevice device, VkSwapchainKHR swapchain);
    std::string extractBufferInfo(VkDevice device, VkBuffer buffer);
    std::string extractImageInfo(VkDevice device, VkImage image);
    std::string extractSemaphoreInfo(VkDevice device, VkSemaphore semaphore);
    std::string extractFenceInfo(VkDevice device, VkFence fence);

    // Member variables
    LogSettings m_settings;
    std::unique_ptr<std::ofstream> m_logFile;
    std::mutex m_logMutex;
    uint32_t m_currentFrame = 0;
    std::string m_currentPhase = "Unknown";
    bool m_initialized = false;
    std::chrono::steady_clock::time_point m_startTime;
};

// Convenience macros for common logging operations
#define VULKAN_LOG_INIT(instance) \
    coregfx::util::VulkanStateLogger::getInstance().logInstanceState(instance, coregfx::util::LogCategory::INITIALIZATION)

#define VULKAN_LOG_DEVICE(device) \
    coregfx::util::VulkanStateLogger::getInstance().logDeviceState(device, coregfx::util::LogCategory::DEVICE_STATE)

#define VULKAN_LOG_MEMORY(device) \
    coregfx::util::VulkanStateLogger::getInstance().logMemoryState(device, coregfx::util::LogCategory::MEMORY_STATE)

#define VULKAN_LOG_EVENT(event) \
    coregfx::util::VulkanStateLogger::getInstance().logEvent(event, coregfx::util::LogCategory::GAME_LOOP)

#define VULKAN_LOG_FRAME_BEGIN(frameNum) \
    coregfx::util::VulkanStateLogger::getInstance().beginFrame(frameNum)

#define VULKAN_LOG_FRAME_END() \
    coregfx::util::VulkanStateLogger::getInstance().endFrame()

#define VULKAN_LOG_PHASE(phase) \
    coregfx::util::VulkanStateLogger::getInstance().setGameLoopPhase(phase)

} // namespace util
} // namespace coregfx

#endif // VULKAN_STATE_LOGGER_HPP_