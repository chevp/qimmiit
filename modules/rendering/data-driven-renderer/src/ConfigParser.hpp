#pragma once

#include "Config.hpp"
#include <string>

namespace datadriven {

/**
 * XML Configuration Parser for arctic.xml
 *
 * Parses the data-driven scene format with:
 * - gRPC backend configuration
 * - Shader templates
 * - Shader instances with uniforms and textures
 * - Camera settings
 */
class ConfigParser {
public:
    /**
     * Load and parse data-driven XML configuration
     * @param xmlPath Path to arctic.xml file
     * @return Parsed configuration with validation status
     */
    static SceneConfig loadConfig(const std::string& xmlPath);

    /**
     * Load and parse arctic.config.xml to initialize ConfigProvider
     * @param xmlPath Path to arctic.config.xml file
     * @return true if successfully loaded and initialized ConfigProvider
     */
    static bool loadCoregfxConfig(const std::string& xmlPath);

private:
    static void parseBackend(void* node, SceneConfig& config);
    static void parseShaderTemplates(void* node, SceneConfig& config);
    static void parseShaderInstances(void* node, SceneConfig& config);
    static void parseShaderInstance(void* instanceNode, SceneConfig& config);
    static void parseCamera(void* node, SceneConfig& config);
    static void parseAssets(void* node, SceneConfig& config);

    static void parseUniforms(void* uniformsNode, ShaderInstanceConfig& instance);
    static void parseTextures(void* texturesNode, ShaderInstanceConfig& instance);
    static void parseTransform(void* transformNode, ShaderInstanceConfig& instance);
    static void parseGeometry(void* node, ShaderInstanceConfig& instance);

    static std::string getElementText(void* node, const char* path, const std::string& defaultValue = "");
    static float getElementFloat(void* node, const char* path, float defaultValue = 0.0f);
    static std::string getAttribute(void* element, const char* attrName, const std::string& defaultValue = "");
};

} // namespace datadriven
