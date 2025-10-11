#pragma once

#include <string>
#include <vector>
#include <map>

namespace datadriven {

struct GrpcBackendConfig {
    std::string address = "127.0.0.1:9081";
    std::string service = "rendering.RendererService";
    std::string stream = "StreamRenderingUpdates";
    bool tls = false;
};

struct ShaderTemplateConfig {
    std::string id;
    std::string name;
    std::string description;
    std::string vertexShader;    // SPIR-V path (e.g., "shaders/pbr.vert.spv")
    std::string fragmentShader;  // SPIR-V path (e.g., "shaders/pbr.frag.spv")
};

struct UniformConfig {
    std::string name;
    std::string type;  // "float", "vec3", "vec4"
    std::string value;
};

struct TextureConfig {
    std::string binding;
    std::string src;
};

struct TransformConfig {
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;
};

struct GeometryConfig {
    std::string meshSrc; // Path to GLTF file (e.g., "models/suzanne-in-box.gltf")
};

struct AssetConfig {
    std::string id;
    std::string src; // File path
};

struct ShaderInstanceConfig {
    std::string id;
    std::string templateId;
    std::vector<UniformConfig> uniforms;
    std::vector<TextureConfig> textures;
    std::vector<GeometryConfig> geometry;
    TransformConfig transform;
};

struct CameraConfig {
    std::string type = "PERSPECTIVE";
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    float posX = 0.0f, posY = 3.0f, posZ = 15.0f;
    float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
};

struct SceneConfig {
    std::string id;
    std::string version;
    std::string name;
    std::string description;

    GrpcBackendConfig backend;
    std::vector<ShaderTemplateConfig> shaderTemplates;
    std::vector<ShaderInstanceConfig> shaderInstances;
    std::vector<AssetConfig> assets;
    CameraConfig camera;

    bool isValid = false;
    std::vector<std::string> errors;
};

} // namespace datadriven
