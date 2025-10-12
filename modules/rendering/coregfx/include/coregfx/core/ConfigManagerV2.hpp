/**
 * ConfigManagerV2.hpp - Protobuf-based Configuration Manager
 *
 * REPLACES the old ConfigManager with a cleaner, protobuf-first approach
 * - Uses cgfx::CoregfxConfig as the SINGLE SOURCE OF TRUTH
 * - Bidirectional XML ↔ Protobuf conversion
 * - Built-in validation
 * - Singleton pattern for global access
 *
 * Usage:
 *   auto& cfg = coregfx::ConfigManagerV2::getInstance();
 *   cfg.loadFromXML("arctic.config.xml");
 *
 *   // Access via protobuf
 *   std::string assetRoot = cfg.getConfig().paths().asset_root();
 *
 *   // Or use convenience methods
 *   if (cfg.isHeadless()) { ... }
 */

#pragma once

#include "cgfx.pb.h"
#include <string>
#include <memory>

namespace coregfx {

class ConfigManagerV2 {
public:
    // Singleton access
    static ConfigManagerV2& getInstance();

    // Delete copy/move constructors
    ConfigManagerV2(const ConfigManagerV2&) = delete;
    ConfigManagerV2& operator=(const ConfigManagerV2&) = delete;

    // Load/Save operations
    bool loadFromXML(const std::string& xml_path);
    bool saveToXML(const std::string& xml_path);
    bool reload();
    void loadDefaults();

    // Access to protobuf config (read-only)
    const cgfx::CoregfxConfig& getConfig() const;

    // Access to protobuf config (mutable - use carefully!)
    cgfx::CoregfxConfig& getMutableConfig();

    // Convenience accessors (type-safe, easy to use)
    const cgfx::ConfigPaths& getPaths() const;
    const cgfx::AppSettings& getSettings() const;
    const cgfx::DebugConfig& getDebug() const;
    const cgfx::CameraConfig& getCamera() const;
    const cgfx::HttpServerConfig& getHttpServer() const;
    const cgfx::StateDumpConfig& getStateDump() const;

    // Common queries (inline for performance)
    bool isHeadless() const;
    bool isStudioMode() const;
    std::string getAssetRoot() const;
    std::string getShaderDir() const;
    int getThreadCount() const;
    cgfx::LogLevel getLogLevel() const;

private:
    ConfigManagerV2();
    ~ConfigManagerV2() = default;

    cgfx::CoregfxConfig config_;
    std::string config_path_;
};

// ============================================================================
// INLINE IMPLEMENTATIONS
// ============================================================================

inline ConfigManagerV2& ConfigManagerV2::getInstance() {
    static ConfigManagerV2 instance;
    return instance;
}

inline const cgfx::CoregfxConfig& ConfigManagerV2::getConfig() const {
    return config_;
}

inline cgfx::CoregfxConfig& ConfigManagerV2::getMutableConfig() {
    return config_;
}

inline const cgfx::ConfigPaths& ConfigManagerV2::getPaths() const {
    return config_.paths();
}

inline const cgfx::AppSettings& ConfigManagerV2::getSettings() const {
    return config_.settings();
}

inline const cgfx::DebugConfig& ConfigManagerV2::getDebug() const {
    return config_.debug();
}

inline const cgfx::CameraConfig& ConfigManagerV2::getCamera() const {
    return config_.camera();
}

inline const cgfx::HttpServerConfig& ConfigManagerV2::getHttpServer() const {
    return config_.http_server();
}

inline const cgfx::StateDumpConfig& ConfigManagerV2::getStateDump() const {
    return config_.state_dump();
}

inline bool ConfigManagerV2::isHeadless() const {
    return config_.settings().headless_mode();
}

inline bool ConfigManagerV2::isStudioMode() const {
    return config_.settings().studio_mode();
}

inline std::string ConfigManagerV2::getAssetRoot() const {
    return config_.paths().asset_root();
}

inline std::string ConfigManagerV2::getShaderDir() const {
    return config_.paths().shader_dir();
}

inline int ConfigManagerV2::getThreadCount() const {
    return config_.variables().thread_count();
}

inline cgfx::LogLevel ConfigManagerV2::getLogLevel() const {
    return config_.debug().log_level();
}

} // namespace coregfx
