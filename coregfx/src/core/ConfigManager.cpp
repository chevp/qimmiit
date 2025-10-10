#include <coregfx/core/ConfigManager.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <iostream>
#include <filesystem>
#include <tinyxml2.h>

namespace coregfx {

AppConfig ConfigManager::loadConfig(const std::string& configPath) {
    AppConfig config;
    config.filePath = configPath;

    ocean::info("Attempting to load configuration from: " + configPath);

    // Check if file exists
    if (!std::filesystem::exists(configPath)) {
        config.errors.push_back("Configuration file not found: " + configPath);
        ocean::error("CONFIG ERROR: File not found - " + configPath);
        return config;
    }

    try {
        ocean::info("Configuration file opened successfully");

        // Load XML file using tinyxml2
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError loadResult = doc.LoadFile(configPath.c_str());

        if (loadResult != tinyxml2::XML_SUCCESS) {
            config.errors.push_back("XML parsing error: Failed to load file");
            ocean::error("XML parsing failed: " + std::string(doc.ErrorStr()));
            return config;
        }

        // Get root element
        tinyxml2::XMLElement* root = doc.RootElement();
        if (!root) {
            config.errors.push_back("XML parsing error: No root element found");
            ocean::error("XML parsing failed: No root element");
            return config;
        }

        // Parse XML sections (each child of root is a section)
        for (tinyxml2::XMLElement* sectionElement = root->FirstChildElement();
             sectionElement != nullptr;
             sectionElement = sectionElement->NextSiblingElement()) {

            std::string sectionName = sectionElement->Name();
            ConfigSection section;

            // Parse all child elements as key-value pairs
            for (tinyxml2::XMLElement* childElement = sectionElement->FirstChildElement();
                 childElement != nullptr;
                 childElement = childElement->NextSiblingElement()) {

                std::string key = childElement->Name();
                const char* textContent = childElement->GetText();
                std::string value = textContent ? textContent : "";

                section.values[key] = value;
            }

            config.sections[sectionName] = section;
        }

        ocean::info("Configuration parsing completed. Found " + std::to_string(config.sections.size()) + " sections");

        // Parse studio mode configuration
        parseStudioConfig(root, config.studioConfig);
    }
    catch (const std::exception& e) {
        config.errors.push_back("Configuration loading error: " + std::string(e.what()));
        ocean::error("Configuration loading failed: " + std::string(e.what()));
    }

    // Validate the loaded configuration
    config.isValid = validateConfig(config);

    if (config.isValid) {
        ocean::info("Configuration validation: PASSED");
    } else {
        ocean::error("Configuration validation: FAILED");
        logConfigErrors(config);
    }

    return config;
}


std::string ConfigManager::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool ConfigManager::validateConfig(const AppConfig& config) {
    std::vector<std::string> validationErrors;

    // Check for at least some essential sections (be flexible about exact names)
    bool hasSettingsLike = (config.sections.find("settings") != config.sections.end() ||
                           config.sections.find("variables") != config.sections.end());
    bool hasDebugLike = (config.sections.find("debug") != config.sections.end() ||
                        config.sections.find("variables") != config.sections.end());

    if (!hasSettingsLike) {
        validationErrors.push_back("Missing configuration section: need either 'settings' or 'variables'");
    }
    if (!hasDebugLike) {
        validationErrors.push_back("Missing debug configuration: need either 'debug' section or LOG_LEVEL in 'variables'");
    }

    // Validate paths if they exist
    if (!validateRequiredPaths(config, validationErrors)) {
        ocean::warn("Path validation failed, but continuing (some paths may be optional)");
    }

    // Validate required settings
    if (!validateRequiredSettings(config, validationErrors)) {
        ocean::error("Required settings validation failed");
    }

    // Add validation errors to config (const_cast is acceptable here for error reporting)
    const_cast<AppConfig&>(config).errors.insert(
        const_cast<AppConfig&>(config).errors.end(),
        validationErrors.begin(),
        validationErrors.end()
    );

    return validationErrors.empty();
}

bool ConfigManager::validateRequiredPaths(const AppConfig& config, std::vector<std::string>& errors) {
    bool allPathsValid = true;

    // Check paths section if it exists
    auto pathsIt = config.sections.find("paths");
    if (pathsIt != config.sections.end()) {
        for (const auto& [key, value] : pathsIt->second.values) {
            // Only validate actual path entries, not version numbers
            if (!value.empty() && key.find("_DIR") != std::string::npos && !isValidPath(value)) {
                errors.push_back("Invalid path for " + key + ": " + value);
                allPathsValid = false;
            }
        }
    }

    return allPathsValid;
}

bool ConfigManager::validateRequiredSettings(const AppConfig& config, std::vector<std::string>& errors) {
    bool allSettingsValid = true;

    // Validate renderer type - it can be in either 'settings' or 'variables' section
    std::string rendererType = getConfigValue(config, "settings", "RENDERER_TYPE");
    if (rendererType.empty()) {
        rendererType = getConfigValue(config, "variables", "RENDERER_TYPE");
    }
    // For now, don't require RENDERER_TYPE as it might be implicit (Vulkan only)
    if (!rendererType.empty() && rendererType != "VULKAN") {
        errors.push_back("Invalid RENDERER_TYPE. Must be 'VULKAN' if specified");
        allSettingsValid = false;
    }

    // Validate log level - check both debug and variables sections
    std::string logLevel = getConfigValue(config, "debug", "log-level");
    if (logLevel.empty()) {
        logLevel = getConfigValue(config, "variables", "log-level");
    }
    if (logLevel.empty()) {
        errors.push_back("Missing log-level (should be in 'debug' or 'variables' section)");
        allSettingsValid = false;
    }

    return allSettingsValid;
}

bool ConfigManager::isValidPath(const std::string& path) {
    if (path.empty()) return false;

    // Remove quotes if present
    std::string cleanPath = path;
    if (cleanPath.front() == '"' && cleanPath.back() == '"') {
        cleanPath = cleanPath.substr(1, cleanPath.length() - 2);
    }

    // Basic validation - just check if it's not obviously invalid
    // Don't require the path to exist, as it might be created later
    return cleanPath.find_first_of("<>|") == std::string::npos &&
           !cleanPath.empty();
}

void ConfigManager::logConfigErrors(const AppConfig& config) {
    ocean::error("=== CONFIGURATION VALIDATION ERRORS ===");
    ocean::error("Configuration file: " + config.filePath);
    ocean::error("Total errors: " + std::to_string(config.errors.size()));

    for (size_t i = 0; i < config.errors.size(); ++i) {
        ocean::error("  " + std::to_string(i + 1) + ". " + config.errors[i]);
    }

    ocean::error("=== END CONFIGURATION ERRORS ===");

    // Also output to console for immediate visibility
    std::cerr << "\n";
    std::cerr << "CONFIGURATION ERROR - Application cannot start\n";
    std::cerr << "File: " << config.filePath << "\n";
    std::cerr << "Errors found:\n";
    for (const auto& error : config.errors) {
        std::cerr << "  • " << error << "\n";
    }
    std::cerr << "\nPlease fix the configuration file and restart the application.\n";
    std::cerr << "\n";
}

std::string ConfigManager::getConfigValue(const AppConfig& config, const std::string& section, const std::string& key, const std::string& defaultValue) {
    auto sectionIt = config.sections.find(section);
    if (sectionIt == config.sections.end()) {
        return defaultValue;
    }

    auto keyIt = sectionIt->second.values.find(key);
    if (keyIt == sectionIt->second.values.end()) {
        return defaultValue;
    }

    return keyIt->second;
}

void ConfigManager::parseStudioConfig(tinyxml2::XMLElement* root, StudioConfig& studioConfig) {
    // Find studio section
    tinyxml2::XMLElement* studioElement = root->FirstChildElement("studio");
    if (!studioElement) {
        ocean::info("No <studio> section found in config - studio mode configuration will use defaults");
        return;
    }

    ocean::info("Parsing studio mode configuration...");

    // Parse electron-apps paths
    tinyxml2::XMLElement* electronApps = studioElement->FirstChildElement("electron-apps");
    if (electronApps) {
        tinyxml2::XMLElement* overlayUi = electronApps->FirstChildElement("arctic-overlay-ui");
        if (overlayUi && overlayUi->GetText()) {
            studioConfig.overlayUiPath = overlayUi->GetText();
            ocean::info("  Overlay UI path: " + studioConfig.overlayUiPath);
        }

        tinyxml2::XMLElement* toolRunner = electronApps->FirstChildElement("arctic-tool-runner");
        if (toolRunner && toolRunner->GetText()) {
            studioConfig.toolRunnerPath = toolRunner->GetText();
            ocean::info("  Tool Runner path: " + studioConfig.toolRunnerPath);
        }
    }

    // Parse auto-launch settings
    tinyxml2::XMLElement* autoLaunch = studioElement->FirstChildElement("auto-launch");
    if (autoLaunch) {
        tinyxml2::XMLElement* overlayUi = autoLaunch->FirstChildElement("overlay-ui");
        if (overlayUi && overlayUi->GetText()) {
            std::string value = overlayUi->GetText();
            studioConfig.autoLaunchOverlay = (value == "true");
            ocean::info("  Auto-launch overlay: " + value);
        }
        // Note: tool-runner no longer auto-launches, only on-demand via overlay UI
    }

    // Parse tools
    tinyxml2::XMLElement* toolsElement = studioElement->FirstChildElement("tools");
    if (toolsElement) {
        for (tinyxml2::XMLElement* tool = toolsElement->FirstChildElement("tool");
             tool != nullptr;
             tool = tool->NextSiblingElement("tool")) {

            ToolInfo toolInfo;

            const char* id = tool->Attribute("id");
            const char* name = tool->Attribute("name");
            if (id) toolInfo.id = id;
            if (name) toolInfo.name = name;

            tinyxml2::XMLElement* url = tool->FirstChildElement("url");
            if (url && url->GetText()) {
                toolInfo.url = url->GetText();
            }

            tinyxml2::XMLElement* localPath = tool->FirstChildElement("local-path");
            if (localPath && localPath->GetText()) {
                toolInfo.localPath = localPath->GetText();
            }

            tinyxml2::XMLElement* route = tool->FirstChildElement("route");
            if (route && route->GetText()) {
                toolInfo.route = route->GetText();
            }

            if (!toolInfo.id.empty() && !toolInfo.url.empty()) {
                studioConfig.tools.push_back(toolInfo);
                ocean::info("  Tool: " + toolInfo.id + " (" + toolInfo.name + ") -> " + toolInfo.url + " [route: " + toolInfo.route + "]");
            }
        }
    }

    // Parse HTTP server port (all renderers use same port, different routes)
    tinyxml2::XMLElement* httpServer = studioElement->FirstChildElement("http-server");
    if (httpServer) {
        tinyxml2::XMLElement* portElement = httpServer->FirstChildElement("port");
        if (portElement && portElement->GetText()) {
            studioConfig.httpServerPort = std::stoi(portElement->GetText());
            ocean::info("  HTTP server port: " + std::to_string(studioConfig.httpServerPort));
        }
    }

    ocean::info("Studio mode configuration parsed successfully (" + std::to_string(studioConfig.tools.size()) + " tools configured)");
}} // namespace coregfx
