/**
 * Nanook Engine - Configuration Manager
 */

#pragma once

#include <string>
#include <unordered_map>

namespace cryo {
namespace engine {

class ConfigManager {
public:
    bool initialize(const std::string& configFile);
    void shutdown();

    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    float getFloat(const std::string& key, float defaultValue = 0.0f) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;

    bool hasKey(const std::string& key) const;
    bool reload();

private:
    std::string configFile_;
    std::unordered_map<std::string, std::string> values_;

    void parseINI(const std::string& content);
    std::string trim(const std::string& str) const;
};

} // namespace engine
} // namespace cryo
