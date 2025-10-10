/**
 * Copyright (C) by chevp
 * Compact SQL-Style Logging Implementation
 */

#include <coregfx/util/compact_logger.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace coregfx {
namespace util {

void CompactLogger::initialize(const std::string& logFile, bool consoleOutput) {
    m_consoleOutput = consoleOutput;
    m_startTime = std::chrono::steady_clock::now();

    if (!logFile.empty()) {
        m_logFile = new std::ofstream(logFile, std::ios::trunc);
        if (!m_logFile->is_open()) {
            std::cerr << "Failed to open compact log file: " << logFile << std::endl;
            delete m_logFile;
            m_logFile = nullptr;
        }
    }

    m_initialized = true;

    // Write SQL schema header
    logComment("Compact SQL-Style Log Started");
    logComment("Schema: elyrion(id, version, author, name, entry_scene, byte_size)");
    logComment("Schema: dependencies(parent_id, id, version)");
    logComment("Schema: assets(id, src, type, size)");
    logComment("Schema: vulkan_state(frame, instance, device, memory_mb, buffers, textures, pipelines)");
}

void CompactLogger::shutdown() {
    if (m_initialized) {
        logComment("Compact SQL-Style Log Ended");
    }

    if (m_logFile) {
        if (m_logFile->is_open()) {
            m_logFile->close();
        }
        delete m_logFile;
        m_logFile = nullptr;
    }

    m_initialized = false;
}

void CompactLogger::logElyrionLoad(const std::string& id, const std::string& version,
                                  const std::string& author, const std::string& name,
                                  const std::string& entryScene, size_t byteSize) {
    std::ostringstream oss;
    oss << "INSERT INTO elyrion (id, version, author, name, entry_scene, byte_size) VALUES ("
        << "'" << escapeString(id) << "', "
        << "'" << escapeString(version) << "', "
        << "'" << escapeString(author) << "', "
        << "'" << escapeString(name) << "', "
        << "'" << escapeString(entryScene) << "', "
        << byteSize << ");";
    writeLog(oss.str());
}

void CompactLogger::logDependency(const std::string& parentId, const std::string& depId,
                                 const std::string& version) {
    std::ostringstream oss;
    oss << "INSERT INTO dependencies (parent_id, id, version) VALUES ("
        << "'" << escapeString(parentId) << "', "
        << "'" << escapeString(depId) << "', "
        << "'" << escapeString(version) << "');";
    writeLog(oss.str());
}

void CompactLogger::logAsset(const std::string& id, const std::string& src,
                            const std::string& type, size_t size) {
    std::ostringstream oss;
    oss << "INSERT INTO assets (id, src, type, size) VALUES ("
        << "'" << escapeString(id) << "', "
        << "'" << escapeString(src) << "', "
        << "'" << escapeString(type) << "', "
        << size << ");";
    writeLog(oss.str());
}

void CompactLogger::logScene(const std::string& id, const std::string& name,
                            int nodeCount, int lightCount) {
    std::ostringstream oss;
    oss << "INSERT INTO scenes (id, name, node_count, light_count) VALUES ("
        << "'" << escapeString(id) << "', "
        << "'" << escapeString(name) << "', "
        << nodeCount << ", " << lightCount << ");";
    writeLog(oss.str());
}

void CompactLogger::logVulkanInstance(VkInstance instance, uint32_t apiVersion,
                                     int layerCount, int extensionCount) {
    std::ostringstream oss;
    oss << "INSERT INTO vulkan_instance (handle, api_version, layers, extensions) VALUES ("
        << "0x" << std::hex << reinterpret_cast<uintptr_t>(instance) << std::dec << ", "
        << VK_VERSION_MAJOR(apiVersion) << "." << VK_VERSION_MINOR(apiVersion) << "." << VK_VERSION_PATCH(apiVersion) << ", "
        << layerCount << ", " << extensionCount << ");";
    writeLog(oss.str());
}

void CompactLogger::logVulkanDevice(VkDevice device, const std::string& deviceName,
                                   uint32_t vendorId, uint32_t deviceId) {
    std::ostringstream oss;
    oss << "INSERT INTO vulkan_device (handle, name, vendor_id, device_id) VALUES ("
        << "0x" << std::hex << reinterpret_cast<uintptr_t>(device) << std::dec << ", "
        << "'" << escapeString(deviceName) << "', "
        << "0x" << std::hex << vendorId << ", 0x" << deviceId << std::dec << ");";
    writeLog(oss.str());
}

void CompactLogger::logVulkanMemory(VkDevice device, size_t totalMemory, size_t usedMemory,
                                   int heapCount) {
    std::ostringstream oss;
    oss << "INSERT INTO vulkan_memory (device, total_mb, used_mb, heap_count) VALUES ("
        << "0x" << std::hex << reinterpret_cast<uintptr_t>(device) << std::dec << ", "
        << (totalMemory / (1024 * 1024)) << ", "
        << (usedMemory / (1024 * 1024)) << ", "
        << heapCount << ");";
    writeLog(oss.str());
}

void CompactLogger::logVulkanResources(VkDevice device, int bufferCount, int imageCount,
                                      int pipelineCount, int descriptorSetCount) {
    std::ostringstream oss;
    oss << "INSERT INTO vulkan_resources (device, buffers, images, pipelines, descriptor_sets) VALUES ("
        << "0x" << std::hex << reinterpret_cast<uintptr_t>(device) << std::dec << ", "
        << bufferCount << ", " << imageCount << ", "
        << pipelineCount << ", " << descriptorSetCount << ");";
    writeLog(oss.str());
}

void CompactLogger::logVulkanFrame(uint32_t frameNum, double deltaTime,
                                  int drawCalls, int triangles) {
    std::ostringstream oss;
    oss << "INSERT INTO vulkan_frame (frame, delta_ms, draw_calls, triangles) VALUES ("
        << frameNum << ", "
        << std::fixed << std::setprecision(2) << (deltaTime * 1000.0) << ", "
        << drawCalls << ", " << triangles << ");";
    writeLog(oss.str());
}

void CompactLogger::logVulkanStateSnapshot(VkInstance instance, VkDevice device,
                                          uint32_t frameNum, const std::string& phase) {
    std::ostringstream oss;
    oss << "SELECT instance=0x" << std::hex << reinterpret_cast<uintptr_t>(instance)
        << ", device=0x" << reinterpret_cast<uintptr_t>(device) << std::dec
        << ", frame=" << frameNum
        << ", phase='" << escapeString(phase) << "'"
        << " FROM vulkan_state;";
    writeLog(oss.str());
}

void CompactLogger::logInsert(const std::string& table, const std::string& columns,
                             const std::string& values) {
    std::ostringstream oss;
    oss << "INSERT INTO " << table << " (" << columns << ") VALUES (" << values << ");";
    writeLog(oss.str());
}

void CompactLogger::logSelect(const std::string& columns, const std::string& table,
                             const std::string& condition) {
    std::ostringstream oss;
    oss << "SELECT " << columns << " FROM " << table;
    if (!condition.empty()) {
        oss << " WHERE " << condition;
    }
    oss << ";";
    writeLog(oss.str());
}

void CompactLogger::logComment(const std::string& comment) {
    std::ostringstream oss;
    oss << "-- [" << getTimestamp() << "] " << comment;
    writeLog(oss.str());
}

void CompactLogger::writeLog(const std::string& message) {
    if (!m_initialized) return;

    if (m_logFile && m_logFile->is_open()) {
        *m_logFile << message << std::endl;
        m_logFile->flush();
    }

    if (m_consoleOutput) {
        std::cout << message << std::endl;
    }
}

std::string CompactLogger::getTimestamp() const {
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

std::string CompactLogger::escapeString(const std::string& str) const {
    std::string result = str;
    // Escape single quotes for SQL
    size_t pos = 0;
    while ((pos = result.find('\'', pos)) != std::string::npos) {
        result.replace(pos, 1, "''");
        pos += 2;
    }
    return result;
}

} // namespace util
} // namespace coregfx