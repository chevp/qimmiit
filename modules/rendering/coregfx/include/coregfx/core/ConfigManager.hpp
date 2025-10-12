#pragma once

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

// Forward declaration (tinyxml2 is in global namespace)
namespace tinyxml2 {
    class XMLElement;
}

namespace coregfx {

struct ConfigSection {
    std::map<std::string, std::string> values;
};

struct ToolInfo {
    std::string id;
    std::string name;
    std::string url;
    std::string localPath;
    std::string route;  // arctic-tool-runner route (e.g., "/io-monitor")
};

struct StudioConfig {
    std::string overlayUiPath;
    std::string toolRunnerPath;
    bool autoLaunchOverlay = true;
    // tool-runner launches on-demand when user clicks tool in overlay UI
    std::vector<ToolInfo> tools;
    int httpServerPort = 52009; // All renderers use same port, different routes
};

struct AppConfig {
    std::map<std::string, ConfigSection> sections;
    StudioConfig studioConfig;
    bool isValid = false;
    std::vector<std::string> errors;
    std::string filePath;
};

class ConfigManager {
public:
    static AppConfig loadConfig(const std::string& configPath = "arctic.config.xml");
    static bool validateConfig(const AppConfig& config);
    static void logConfigErrors(const AppConfig& config);
    static std::string getConfigValue(const AppConfig& config, const std::string& section, const std::string& key, const std::string& defaultValue = "");

private:
    static std::string trim(const std::string& str);
    static bool isValidPath(const std::string& path);
    static bool validateRequiredPaths(const AppConfig& config, std::vector<std::string>& errors);
    static bool validateRequiredSettings(const AppConfig& config, std::vector<std::string>& errors);
    static void parseStudioConfig(tinyxml2::XMLElement* root, StudioConfig& studioConfig);
};

} // namespace coregfx