/**
 * Copyright (C) by chevp
 * JSON Event Logger Implementation
 */

#include <coregfx/util/json_event_logger.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace coregfx {
namespace util {

bool JsonEventLogger::initialize(const std::string& logFile, bool consoleOutput) {
    std::lock_guard<std::mutex> lock(m_logMutex);

    m_consoleOutput = consoleOutput;
    m_startTime = std::chrono::steady_clock::now();

    if (!logFile.empty()) {
        m_logFile = std::make_unique<std::ofstream>(logFile, std::ios::trunc);
        if (!m_logFile->is_open()) {
            std::cerr << "Failed to open JSON event log file: " << logFile << std::endl;
            return false;
        }
    }

    m_initialized = true;

    // Write initialization event
    std::map<std::string, std::string> initData;
    initData["message"] = "JSON Event Logger Started";
    initData["timestamp"] = getTimestamp();
    initData["format"] = "ndjson";
    initData["log_file"] = logFile.empty() ? "console_only" : logFile;
    logEvent("logger_init", initData);

    return true;
}

void JsonEventLogger::shutdown() {
    std::lock_guard<std::mutex> lock(m_logMutex);

    if (m_initialized) {
        std::map<std::string, std::string> shutdownData;
        shutdownData["message"] = "JSON Event Logger Shutdown";
        shutdownData["timestamp"] = getTimestamp();
        logEvent("logger_shutdown", shutdownData);
    }

    if (m_logFile && m_logFile->is_open()) {
        m_logFile->close();
    }

    m_initialized = false;
}

void JsonEventLogger::logEvent(const std::string& eventType, const std::map<std::string, std::string>& data) {
    if (!m_initialized) return;

    std::map<std::string, std::string> eventData = data;
    eventData["event_type"] = eventType;
    eventData["timestamp"] = getTimestamp();

    std::string jsonEvent = createJsonObject(eventData);
    writeJsonEvent(jsonEvent);
}

void JsonEventLogger::logElyrionLoad(const std::string& id, const std::string& version,
                                    const std::string& author, const std::string& name,
                                    const std::string& entryScene, size_t byteSize) {
    std::map<std::string, std::string> data;
    data["id"] = id;
    data["version"] = version;
    data["author"] = author;
    data["name"] = name;
    data["entry_scene"] = entryScene;
    data["byte_size"] = std::to_string(byteSize);
    logEvent("elyrion_load", data);
}

void JsonEventLogger::logAsset(const std::string& id, const std::string& src,
                              const std::string& type, size_t size) {
    std::map<std::string, std::string> data;
    data["id"] = id;
    data["src"] = src;
    data["type"] = type;
    data["size"] = std::to_string(size);
    logEvent("asset", data);
}

void JsonEventLogger::logSceneDetail(const std::string& id, const std::string& name,
                                    int totalEntities) {
    std::map<std::string, std::string> data;
    data["id"] = id;
    data["name"] = name;
    data["total_entities"] = std::to_string(totalEntities);
    logEvent("scene_detail", data);
}

void JsonEventLogger::logEntity(int index, const std::string& id, const std::string& name,
                               const std::vector<std::string>& classes) {
    std::map<std::string, std::string> data;
    data["index"] = std::to_string(index);
    data["id"] = id;
    data["name"] = name;

    // Convert classes vector to comma-separated string
    std::ostringstream classesStr;
    for (size_t i = 0; i < classes.size(); ++i) {
        if (i > 0) classesStr << ",";
        classesStr << classes[i];
    }
    data["classes"] = classesStr.str();
    logEvent("entity", data);
}

void JsonEventLogger::logCamera(float posX, float posY, float posZ,
                               float rotX, float rotY, float rotZ,
                               const std::string& projection) {
    std::map<std::string, std::string> data;
    data["pos_x"] = std::to_string(posX);
    data["pos_y"] = std::to_string(posY);
    data["pos_z"] = std::to_string(posZ);
    data["rot_x"] = std::to_string(rotX);
    data["rot_y"] = std::to_string(rotY);
    data["rot_z"] = std::to_string(rotZ);
    data["projection"] = projection;
    logEvent("camera", data);
}

void JsonEventLogger::logVulkanInstance(const std::string& version, int layerCount,
                                       int extensionCount) {
    std::map<std::string, std::string> data;
    data["version"] = version;
    data["layer_count"] = std::to_string(layerCount);
    data["extension_count"] = std::to_string(extensionCount);
    logEvent("vulkan_instance", data);
}

void JsonEventLogger::logVulkanDevice(const std::string& name, const std::string& version,
                                     const std::string& driver) {
    std::map<std::string, std::string> data;
    data["name"] = name;
    data["version"] = version;
    data["driver"] = driver;
    logEvent("vulkan_device", data);
}

void JsonEventLogger::logVulkanMemory(size_t allocatedBytes, int allocationCount,
                                     size_t peakBytes) {
    std::map<std::string, std::string> data;
    data["allocated_bytes"] = std::to_string(allocatedBytes);
    data["allocation_count"] = std::to_string(allocationCount);
    data["peak_bytes"] = std::to_string(peakBytes);
    data["allocated_mb"] = std::to_string(allocatedBytes / (1024 * 1024));
    data["peak_mb"] = std::to_string(peakBytes / (1024 * 1024));
    logEvent("vulkan_memory", data);
}

void JsonEventLogger::logVulkanPipeline(const std::string& name, const std::string& stages,
                                       bool active) {
    std::map<std::string, std::string> data;
    data["name"] = name;
    data["stages"] = stages;
    data["active"] = active ? "true" : "false";
    logEvent("vulkan_pipeline", data);
}

void JsonEventLogger::logVulkanRenderPass(const std::string& name, int attachmentCount,
                                         bool active) {
    std::map<std::string, std::string> data;
    data["name"] = name;
    data["attachment_count"] = std::to_string(attachmentCount);
    data["active"] = active ? "true" : "false";
    logEvent("vulkan_render_pass", data);
}

void JsonEventLogger::logPerformanceMetrics(double frameTimeMs, int drawCalls,
                                           int triangles, double gpuMemoryMB) {
    std::map<std::string, std::string> data;
    data["frame_time_ms"] = std::to_string(frameTimeMs);
    data["draw_calls"] = std::to_string(drawCalls);
    data["triangles"] = std::to_string(triangles);
    data["gpu_memory_mb"] = std::to_string(gpuMemoryMB);
    data["fps"] = std::to_string(1000.0 / frameTimeMs);
    logEvent("performance_metrics", data);
}

void JsonEventLogger::logStateTransition(const std::string& fromState,
                                        const std::string& toState,
                                        const std::string& reason) {
    std::map<std::string, std::string> data;
    data["from_state"] = fromState;
    data["to_state"] = toState;
    data["reason"] = reason;
    logEvent("state_transition", data);
}

void JsonEventLogger::writeJsonEvent(const std::string& jsonString) {
    if (!m_initialized) return;

    std::lock_guard<std::mutex> lock(m_logMutex);

    if (m_logFile && m_logFile->is_open()) {
        *m_logFile << jsonString << std::endl;
        m_logFile->flush();
    }

    if (m_consoleOutput) {
        std::cout << jsonString << std::endl;
    }
}

std::string JsonEventLogger::getTimestamp() const {
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

std::string JsonEventLogger::escapeJsonString(const std::string& str) const {
    std::string result;
    result.reserve(str.length() + 10); // Reserve some extra space for escaping

    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c < 0x20) {
                    // Control characters - escape as \uXXXX
                    std::ostringstream oss;
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    result += oss.str();
                } else {
                    result += c;
                }
                break;
        }
    }

    return result;
}

std::string JsonEventLogger::createJsonObject(const std::map<std::string, std::string>& fields) const {
    std::ostringstream oss;
    oss << "{";

    bool first = true;
    for (const auto& field : fields) {
        if (!first) oss << ",";
        oss << "\"" << escapeJsonString(field.first) << "\":"
            << "\"" << escapeJsonString(field.second) << "\"";
        first = false;
    }

    oss << "}";
    return oss.str();
}

} // namespace util
} // namespace coregfx