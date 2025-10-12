/**
 * ConfigManagerV2.cpp - Implementation of Protobuf-based Config Manager
 */

#include <coregfx/core/ConfigManagerV2.hpp>
#include <coregfx/core/ConfigConverter.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <filesystem>

namespace coregfx {

ConfigManagerV2::ConfigManagerV2() {
    // Load defaults on construction
    loadDefaults();
}

void ConfigManagerV2::loadDefaults() {
    config::applyDefaults(config_);
    ocean_log("ConfigManagerV2", "Applied default configuration");
}

bool ConfigManagerV2::loadFromXML(const std::string& xml_path) {
    try {
        // Parse XML into protobuf
        config_ = config::loadFromXML(xml_path);

        // Apply defaults for any missing values
        config::applyDefaults(config_);

        // Validate configuration
        auto validation = config::validate(config_);
        if (!validation.valid()) {
            ocean_log("ConfigManagerV2", "Config validation failed:");
            for (const auto& err : validation.errors()) {
                if (err.severity() == cgfx::CONFIG_ERROR_ERROR || err.severity() == cgfx::CONFIG_ERROR_FATAL) {
                    ocean_log("ConfigManagerV2", "  ERROR: [" + err.field_path() + "] " + err.error_message());
                }
            }
            return false;
        }

        // Log warnings
        bool hasWarnings = false;
        for (const auto& err : validation.errors()) {
            if (err.severity() == cgfx::CONFIG_ERROR_WARNING) {
                if (!hasWarnings) {
                    ocean_log("ConfigManagerV2", "Config loaded with warnings:");
                    hasWarnings = true;
                }
                ocean_log("ConfigManagerV2", "  WARNING: [" + err.field_path() + "] " + err.error_message());
            }
        }

        config_path_ = xml_path;
        ocean_log("ConfigManagerV2", "✓ Config loaded: " + xml_path);

        // Log key settings
        ocean_log("ConfigManagerV2", "  Asset Root: " + config_.paths().asset_root());
        ocean_log("ConfigManagerV2", "  Headless: " + std::string(config_.settings().headless_mode() ? "true" : "false"));
        ocean_log("ConfigManagerV2", "  HTTP Port: " + std::to_string(config_.http_server().port()));

        return true;

    } catch (const std::exception& e) {
        ocean_log("ConfigManagerV2", std::string("✗ Failed to load config: ") + e.what());
        return false;
    }
}

bool ConfigManagerV2::saveToXML(const std::string& xml_path) {
    try {
        config::saveToXML(config_, xml_path);
        ocean_log("ConfigManagerV2", "✓ Config saved: " + xml_path);
        return true;
    } catch (const std::exception& e) {
        ocean_log("ConfigManagerV2", std::string("✗ Failed to save config: ") + e.what());
        return false;
    }
}

bool ConfigManagerV2::reload() {
    if (config_path_.empty()) {
        ocean_log("ConfigManagerV2", "No config path set, cannot reload");
        return false;
    }
    ocean_log("ConfigManagerV2", "Reloading config from: " + config_path_);
    return loadFromXML(config_path_);
}

} // namespace coregfx
