/**
 * Nanook Engine - Configuration Manager Implementation
 */

#include "ConfigManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace cryo {
namespace engine {

bool ConfigManager::initialize(const std::string& configFile) {
    configFile_ = configFile;

    std::ifstream file(configFile);
    if (!file.good()) {
        std::cerr << "Configuration file not found: " << configFile << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Parse INI-style config into key-value map
    parseINI(content);

    return true;
}

void ConfigManager::shutdown() {
    values_.clear();
}

std::string ConfigManager::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = values_.find(key);
    return (it != values_.end()) ? it->second : defaultValue;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) const {
    auto it = values_.find(key);
    if (it != values_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

float ConfigManager::getFloat(const std::string& key, float defaultValue) const {
    auto it = values_.find(key);
    if (it != values_.end()) {
        try {
            return std::stof(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const {
    auto it = values_.find(key);
    if (it != values_.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return (value == "true" || value == "1" || value == "yes");
    }
    return defaultValue;
}

bool ConfigManager::hasKey(const std::string& key) const {
    return values_.find(key) != values_.end();
}

bool ConfigManager::reload() {
    values_.clear();
    return initialize(configFile_);
}

void ConfigManager::parseINI(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    std::string currentSection;

    while (std::getline(stream, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Check for section headers [SectionName]
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            currentSection = trim(currentSection);
            continue;
        }

        // Parse key=value pairs
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));

            // Prepend section name to key if in a section
            if (!currentSection.empty()) {
                key = currentSection + "." + key;
            }

            values_[key] = value;
        }
    }
}

std::string ConfigManager::trim(const std::string& str) const {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

} // namespace engine
} // namespace cryo