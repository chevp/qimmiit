#pragma once

#include <string>
#include <map>

/**
 * Global configuration provider for accessing configuration values throughout the coregfx module
 * This allows the coregfx static library to access configuration values set by the main application
 */
class ConfigProvider {
public:
    static void initialize(const std::map<std::string, std::string>& pathValues);
    static std::string getAssetRoot();
    static std::string getShaderDir();
    static std::string getPath(const std::string& key, const std::string& defaultValue = "");

private:
    static std::map<std::string, std::string> paths_;
    static bool initialized_;
};