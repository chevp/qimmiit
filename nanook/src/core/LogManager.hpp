/**
 * Nanook Engine - Log Manager
 */

#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace cryo {
namespace engine {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class LogManager {
public:
    bool initialize(const std::string& logLevel, const std::vector<std::string>& outputs);
    void shutdown();

    void log(LogLevel level, const std::string& module, const std::string& message);
    void debug(const std::string& module, const std::string& message);
    void info(const std::string& module, const std::string& message);
    void warn(const std::string& module, const std::string& message);
    void error(const std::string& module, const std::string& message);

private:
    LogLevel logLevel_ = LogLevel::INFO;
    std::vector<std::string> logOutputs_;

    std::string getCurrentTimestamp() const;
    std::string logLevelToString(LogLevel level) const;
};

} // namespace engine
} // namespace cryo
