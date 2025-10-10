/**
 * Copyright (C) by chevp
 * JSON Event Logger for Machine-Readable Structured Logging
 */

#pragma once

#include <string>
#include <chrono>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>
#include <map>

namespace coregfx {
namespace util {

/**
 * JSON Event Logger for structured, machine-readable logging
 * Outputs newline-delimited JSON (NDJSON) for easy parsing
 * Uses simple JSON generation to avoid external dependencies
 */
class JsonEventLogger {
public:
    static JsonEventLogger& getInstance() {
        static JsonEventLogger instance;
        return instance;
    }

    // Initialization and configuration
    bool initialize(const std::string& logFile = "", bool consoleOutput = true);
    void shutdown();

    // Core event logging methods
    void logEvent(const std::string& eventType, const std::map<std::string, std::string>& data);

    // Specific Vulkan state events
    void logElyrionLoad(const std::string& id, const std::string& version,
                       const std::string& author, const std::string& name,
                       const std::string& entryScene, size_t byteSize);

    void logAsset(const std::string& id, const std::string& src,
                 const std::string& type = "gltf", size_t size = 0);

    void logSceneDetail(const std::string& id, const std::string& name,
                       int totalEntities);

    void logEntity(int index, const std::string& id, const std::string& name = "",
                  const std::vector<std::string>& classes = {});

    void logCamera(float posX, float posY, float posZ,
                  float rotX, float rotY, float rotZ,
                  const std::string& projection = "perspective");

    void logVulkanInstance(const std::string& version, int layerCount,
                          int extensionCount);

    void logVulkanDevice(const std::string& name, const std::string& version,
                        const std::string& driver);

    void logVulkanMemory(size_t allocatedBytes, int allocationCount,
                        size_t peakBytes);

    void logVulkanPipeline(const std::string& name, const std::string& stages,
                          bool active);

    void logVulkanRenderPass(const std::string& name, int attachmentCount,
                           bool active);

    void logPerformanceMetrics(double frameTimeMs, int drawCalls,
                             int triangles, double gpuMemoryMB);

    void logStateTransition(const std::string& fromState,
                          const std::string& toState,
                          const std::string& reason = "");

private:
    JsonEventLogger() = default;
    ~JsonEventLogger() { shutdown(); }

    JsonEventLogger(const JsonEventLogger&) = delete;
    JsonEventLogger& operator=(const JsonEventLogger&) = delete;

    void writeJsonEvent(const std::string& jsonString);
    std::string getTimestamp() const;
    std::string escapeJsonString(const std::string& str) const;
    std::string createJsonObject(const std::map<std::string, std::string>& fields) const;

    bool m_initialized = false;
    bool m_consoleOutput = true;
    std::unique_ptr<std::ofstream> m_logFile;
    std::chrono::steady_clock::time_point m_startTime;
    std::mutex m_logMutex;
};

} // namespace util
} // namespace coregfx