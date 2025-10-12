#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>

namespace coregfx {

// Forward declaration for Config structures - these should be moved here too
namespace arctic {

struct GrpcBackendConfig {
    std::string address = "127.0.0.1:9081";
    std::string serviceName = "rendering.RendererService";
    bool useTls = false;
};

struct ShaderTemplateConfig {
    std::string id;
    std::string name;
    std::string vertexShader;
    std::string fragmentShader;
};

struct UniformConfig {
    std::string name;
    std::string type;  // "float", "vec3", "vec4"
    std::string value;
};

struct TextureBindingConfig {
    std::string name;
    std::string path;
};

struct ShaderInstanceConfig {
    std::string id;
    std::string templateId;
    std::vector<UniformConfig> uniforms;
    std::vector<TextureBindingConfig> textures;
    std::string geometrySrc;  // asset:// URI or "none"
};

struct AssetConfig {
    std::string type;  // "mesh", "texture", etc.
    std::string id;
    std::string src;   // asset:// URI
};

struct SceneConfig {
    std::string id;
    std::string name;
    std::string version;
    GrpcBackendConfig backend;
    std::vector<ShaderTemplateConfig> shaderTemplates;
    std::vector<ShaderInstanceConfig> shaderInstances;
    std::vector<AssetConfig> assets;
    glm::vec3 cameraPosition = glm::vec3(0, 3, 15);
    glm::vec3 cameraRotation = glm::vec3(0, 0, 0);
    float cameraFov = 60.0f;
    bool isValid = false;
    std::vector<std::string> errors;
};

} // namespace arctic

/**
 * XML Configuration Parser for arctic.xml (data-driven shader scenes)
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
