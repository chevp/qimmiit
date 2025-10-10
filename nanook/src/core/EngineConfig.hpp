/**
 * Cryo Engine - Engine Configuration
 */

#pragma once

#include <string>
#include <vector>

// Forward declare GLFW types
struct GLFWwindow;

namespace cryo {
namespace engine {

/**
 * Engine configuration structure
 */
struct EngineConfig {
    // Window handle (provided by Nunaq)
    GLFWwindow* window = nullptr;

    // Configuration file (optional - leave empty to skip file-based config)
    std::string configFile = "";

    // Logging
    std::string logLevel = "INFO";
    std::vector<std::string> logOutputs = {"console"};  // "console", "file"

    // Threading
    size_t threadCount = 4;

    // Memory
    size_t memoryPoolSizeMB = 512;

    // Module configuration
    bool enableRenderer = true;
    bool enableCache = true;
    bool enableAsset = true;
    bool enableHttp = false;
    bool enableCompute = false;

    // Validation
    bool isValid() const {
        return threadCount > 0 && threadCount <= 64 &&
               memoryPoolSizeMB > 0 && memoryPoolSizeMB <= 8192;
    }
};

} // namespace engine
} // namespace cryo