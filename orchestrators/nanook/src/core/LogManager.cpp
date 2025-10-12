/**
 * Cryo Engine - Log Manager Implementation
 */

#include "LogManager.hpp"
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

namespace cryo {
namespace engine {

bool LogManager::initialize(const std::string& logLevel, const std::vector<std::string>& outputs) {
    // Parse log level
    if (logLevel == "DEBUG") {
        logLevel_ = LogLevel::DEBUG;
    } else if (logLevel == "INFO") {
        logLevel_ = LogLevel::INFO;
    } else if (logLevel == "WARN") {
        logLevel_ = LogLevel::WARN;
    } else if (logLevel == "ERROR") {
        logLevel_ = LogLevel::ERROR;
    }

    logOutputs_ = outputs;

    return true;
}

void LogManager::shutdown() {
    // Nothing to clean up
}

void LogManager::log(LogLevel level, const std::string& module, const std::string& message) {
    if (level < logLevel_) return;

    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = logLevelToString(level);

    std::string formatted = "[" + timestamp + "] [" + levelStr + "] [" + module + "] " + message;

    // Output to console
    if (std::find(logOutputs_.begin(), logOutputs_.end(), "console") != logOutputs_.end()) {
        std::cout << formatted << std::endl;
    }

    // Output to file
    if (std::find(logOutputs_.begin(), logOutputs_.end(), "file") != logOutputs_.end()) {
        std::ofstream logFile("cryo-engine.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << formatted << std::endl;
        }
    }
}

void LogManager::debug(const std::string& module, const std::string& message) {
    log(LogLevel::DEBUG, module, message);
}

void LogManager::info(const std::string& module, const std::string& message) {
    log(LogLevel::INFO, module, message);
}

void LogManager::warn(const std::string& module, const std::string& message) {
    log(LogLevel::WARN, module, message);
}

void LogManager::error(const std::string& module, const std::string& message) {
    log(LogLevel::ERROR, module, message);
}

std::string LogManager::getCurrentTimestamp() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string LogManager::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

} // namespace engine
} // namespace cryo