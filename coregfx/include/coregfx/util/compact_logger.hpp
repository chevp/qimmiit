/**
 * Copyright (C) by chevp
 * Compact SQL-Style Logging System for Object and Vulkan State Tracking
 */

#pragma once

#ifndef COMPACT_LOGGER_HPP_
#define COMPACT_LOGGER_HPP_

#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <vulkan/vulkan.h>

namespace coregfx {
namespace util {

class CompactLogger {
public:
    static CompactLogger& getInstance() {
        static CompactLogger instance;
        return instance;
    }

    // Delete copy constructor and assignment
    CompactLogger(const CompactLogger&) = delete;
    CompactLogger& operator=(const CompactLogger&) = delete;

    // Initialize compact logger
    void initialize(const std::string& logFile = "", bool consoleOutput = true);
    void shutdown();

    // Object Logging (SQL-style)
    void logElyrionLoad(const std::string& id, const std::string& version,
                       const std::string& author, const std::string& name,
                       const std::string& entryScene, size_t byteSize);

    void logDependency(const std::string& parentId, const std::string& depId,
                      const std::string& version);

    void logAsset(const std::string& id, const std::string& src,
                 const std::string& type, size_t size = 0);

    void logScene(const std::string& id, const std::string& name,
                 int nodeCount, int lightCount);

    // Vulkan State Logging (Compact SQL-style)
    void logVulkanInstance(VkInstance instance, uint32_t apiVersion,
                          int layerCount, int extensionCount);

    void logVulkanDevice(VkDevice device, const std::string& deviceName,
                        uint32_t vendorId, uint32_t deviceId);

    void logVulkanMemory(VkDevice device, size_t totalMemory, size_t usedMemory,
                        int heapCount);

    void logVulkanResources(VkDevice device, int bufferCount, int imageCount,
                           int pipelineCount, int descriptorSetCount);

    void logVulkanFrame(uint32_t frameNum, double deltaTime,
                       int drawCalls, int triangles);

    // Compact state snapshot
    void logVulkanStateSnapshot(VkInstance instance, VkDevice device,
                               uint32_t frameNum, const std::string& phase);

    // Generic SQL-style insert/select helpers
    void logInsert(const std::string& table, const std::string& columns,
                  const std::string& values);

    void logSelect(const std::string& columns, const std::string& table,
                  const std::string& condition = "");

    void logComment(const std::string& comment);

private:
    CompactLogger() = default;
    ~CompactLogger() { shutdown(); }

    void writeLog(const std::string& message);
    std::string getTimestamp() const;
    std::string escapeString(const std::string& str) const;

    std::ofstream* m_logFile = nullptr;
    bool m_consoleOutput = true;
    bool m_initialized = false;
    std::chrono::steady_clock::time_point m_startTime;
};

// Convenience macros for compact logging
#define COMPACT_LOG_ELYRION(id, ver, author, name, entry, size) \
    coregfx::util::CompactLogger::getInstance().logElyrionLoad(id, ver, author, name, entry, size)

#define COMPACT_LOG_VULKAN_FRAME(frame, dt, draws, tris) \
    coregfx::util::CompactLogger::getInstance().logVulkanFrame(frame, dt, draws, tris)

#define COMPACT_LOG_VULKAN_STATE(instance, device, frame, phase) \
    coregfx::util::CompactLogger::getInstance().logVulkanStateSnapshot(instance, device, frame, phase)

#define COMPACT_LOG_COMMENT(text) \
    coregfx::util::CompactLogger::getInstance().logComment(text)

} // namespace util
} // namespace coregfx

#endif // COMPACT_LOGGER_HPP_