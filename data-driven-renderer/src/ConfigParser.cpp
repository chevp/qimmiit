#include "ConfigParser.hpp"
#include <coregfx/core/ocean_log.hpp>
#include <coregfx/core/ConfigProvider.hpp>
#include <tinyxml2.h>
#include <sstream>
#include <map>

namespace datadriven {

// Compact XML-style logging helper
static void compactLog(const std::string& category, const std::string& data) {
    ocean::info("COMPACT: " + category + " - " + data);
}

SceneConfig ConfigParser::loadConfig(const std::string& xmlPath) {
    SceneConfig config;

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.c_str()) != tinyxml2::XML_SUCCESS) {
        config.isValid = false;
        config.errors.push_back("Failed to load XML file: " + xmlPath);
        ocean::error("Failed to load data-driven XML: " + xmlPath);
        return config;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("data-driven-scene");
    if (!root) {
        config.isValid = false;
        config.errors.push_back("Missing root element <data-driven-scene>");
        ocean::error("Invalid XML: missing <data-driven-scene> root element");
        return config;
    }

    // Parse root attributes
    config.id = getAttribute(root, "id", "unknown");
    config.version = getAttribute(root, "version", "1.0.0");

    // Parse name and description
    config.name = getElementText(root, "name", "Unnamed Scene");
    config.description = getElementText(root, "description", "");

    compactLog("Scene", "id=" + config.id + ", name=" + config.name + ", version=" + config.version);

    // Parse backend configuration
    tinyxml2::XMLElement* backend = root->FirstChildElement("backend");
    if (backend) {
        parseBackend(backend, config);
    } else {
        config.errors.push_back("Warning: No <backend> configuration found");
    }

    // Parse shader templates
    tinyxml2::XMLElement* templates = root->FirstChildElement("shader-templates");
    if (templates) {
        parseShaderTemplates(templates, config);
    }

    // Parse shader instances
    tinyxml2::XMLElement* instances = root->FirstChildElement("shader-instances");
    if (instances) {
        parseShaderInstances(instances, config);
    }

    // Parse assets
    tinyxml2::XMLElement* assets = root->FirstChildElement("assets");
    if (assets) {
        parseAssets(assets, config);
    }

    // Parse camera
    tinyxml2::XMLElement* camera = root->FirstChildElement("camera");
    if (camera) {
        parseCamera(camera, config);
    }

    config.isValid = config.errors.empty() ||
                     (config.errors.size() == 1 && config.errors[0].find("Warning") != std::string::npos);

    compactLog("Summary", "templates=" + std::to_string(config.shaderTemplates.size()) +
               ", instances=" + std::to_string(config.shaderInstances.size()) +
               ", assets=" + std::to_string(config.assets.size()) +
               ", valid=" + (config.isValid ? "true" : "false"));

    return config;
}

void ConfigParser::parseBackend(void* node, SceneConfig& config) {
    tinyxml2::XMLElement* backend = static_cast<tinyxml2::XMLElement*>(node);
    tinyxml2::XMLElement* grpc = backend->FirstChildElement("grpc");

    if (grpc) {
        config.backend.address = getElementText(grpc, "address", "127.0.0.1:9081");
        config.backend.service = getElementText(grpc, "service", "rendering.RendererService");
        config.backend.stream = getElementText(grpc, "stream", "StreamRenderingUpdates");

        std::string tlsStr = getElementText(grpc, "tls", "false");
        config.backend.tls = (tlsStr == "true");

        compactLog("Backend", "address=" + config.backend.address + ", service=" + config.backend.service + ", tls=" + (config.backend.tls ? "true" : "false"));
    }
}

void ConfigParser::parseShaderTemplates(void* node, SceneConfig& config) {
    tinyxml2::XMLElement* templatesNode = static_cast<tinyxml2::XMLElement*>(node);

    for (tinyxml2::XMLElement* tmpl = templatesNode->FirstChildElement("template"); tmpl; tmpl = tmpl->NextSiblingElement("template")) {
        ShaderTemplateConfig templateConfig;
        templateConfig.id = getAttribute(tmpl, "id", "");

        // Parse embedded template (XML format, no external JSON files)
        tinyxml2::XMLElement* nameElem = tmpl->FirstChildElement("name");
        if (nameElem && nameElem->GetText()) {
            templateConfig.name = nameElem->GetText();
        }

        tinyxml2::XMLElement* descElem = tmpl->FirstChildElement("description");
        if (descElem && descElem->GetText()) {
            templateConfig.description = descElem->GetText();
        }

        // Parse shader paths (SPIR-V files)
        tinyxml2::XMLElement* shadersElem = tmpl->FirstChildElement("shaders");
        if (shadersElem) {
            tinyxml2::XMLElement* vertElem = shadersElem->FirstChildElement("vertex");
            if (vertElem) {
                templateConfig.vertexShader = getAttribute(vertElem, "src", "");
            }

            tinyxml2::XMLElement* fragElem = shadersElem->FirstChildElement("fragment");
            if (fragElem) {
                templateConfig.fragmentShader = getAttribute(fragElem, "src", "");
            }
        }

        if (!templateConfig.id.empty()) {
            config.shaderTemplates.push_back(templateConfig);
            compactLog("Template", "id=" + templateConfig.id + ", name=" + templateConfig.name + ", vert=" + templateConfig.vertexShader + ", frag=" + templateConfig.fragmentShader);
        }
    }
}

void ConfigParser::parseShaderInstances(void* node, SceneConfig& config) {
    tinyxml2::XMLElement* instancesNode = static_cast<tinyxml2::XMLElement*>(node);

    for (tinyxml2::XMLElement* inst = instancesNode->FirstChildElement("instance"); inst; inst = inst->NextSiblingElement("instance")) {
        parseShaderInstance(inst, config);
    }
}

void ConfigParser::parseShaderInstance(void* instanceNode, SceneConfig& config) {
    tinyxml2::XMLElement* inst = static_cast<tinyxml2::XMLElement*>(instanceNode);

    ShaderInstanceConfig instance;
    instance.id = getAttribute(inst, "id", "");
    instance.templateId = getAttribute(inst, "template", "");

    if (instance.id.empty() || instance.templateId.empty()) {
        config.errors.push_back("Shader instance missing id or template");
        return;
    }

    // Parse uniforms
    tinyxml2::XMLElement* uniforms = inst->FirstChildElement("uniforms");
    if (uniforms) {
        parseUniforms(uniforms, instance);
    }

    // Parse textures
    tinyxml2::XMLElement* textures = inst->FirstChildElement("textures");
    if (textures) {
        parseTextures(textures, instance);
    }

    // Parse geometry
    tinyxml2::XMLElement* geometry = inst->FirstChildElement("geometry");
    if (geometry) {
        parseGeometry(geometry, instance);
    }

    // Parse transform
    tinyxml2::XMLElement* transform = inst->FirstChildElement("transform");
    if (transform) {
        parseTransform(transform, instance);
    }

    config.shaderInstances.push_back(instance);

    std::string geomInfo = instance.geometry.empty() ? "none" : instance.geometry[0].meshSrc;
    compactLog("Instance", "id=" + instance.id + ", template=" + instance.templateId +
               ", uniforms=" + std::to_string(instance.uniforms.size()) +
               ", textures=" + std::to_string(instance.textures.size()) +
               ", geometry=" + geomInfo);
}

void ConfigParser::parseUniforms(void* uniformsNode, ShaderInstanceConfig& instance) {
    tinyxml2::XMLElement* uniformsElem = static_cast<tinyxml2::XMLElement*>(uniformsNode);

    for (tinyxml2::XMLElement* uniform = uniformsElem->FirstChildElement("uniform"); uniform; uniform = uniform->NextSiblingElement("uniform")) {
        UniformConfig uniformConfig;
        uniformConfig.name = getAttribute(uniform, "name", "");
        uniformConfig.type = getAttribute(uniform, "type", "float");
        uniformConfig.value = getAttribute(uniform, "value", "0.0");

        if (!uniformConfig.name.empty()) {
            instance.uniforms.push_back(uniformConfig);
        }
    }
}

void ConfigParser::parseTextures(void* texturesNode, ShaderInstanceConfig& instance) {
    tinyxml2::XMLElement* texturesElem = static_cast<tinyxml2::XMLElement*>(texturesNode);

    for (tinyxml2::XMLElement* texture = texturesElem->FirstChildElement("texture"); texture; texture = texture->NextSiblingElement("texture")) {
        TextureConfig textureConfig;
        textureConfig.binding = getAttribute(texture, "binding", "");
        textureConfig.src = getAttribute(texture, "src", "");

        if (!textureConfig.binding.empty() && !textureConfig.src.empty()) {
            instance.textures.push_back(textureConfig);
        }
    }
}

void ConfigParser::parseTransform(void* transformNode, ShaderInstanceConfig& instance) {
    tinyxml2::XMLElement* transform = static_cast<tinyxml2::XMLElement*>(transformNode);

    // Position
    tinyxml2::XMLElement* position = transform->FirstChildElement("position");
    if (position) {
        instance.transform.posX = getElementFloat(position, nullptr, 0.0f);
        if (const char* x = position->Attribute("x")) instance.transform.posX = std::stof(x);
        if (const char* y = position->Attribute("y")) instance.transform.posY = std::stof(y);
        if (const char* z = position->Attribute("z")) instance.transform.posZ = std::stof(z);
    }

    // Rotation
    tinyxml2::XMLElement* rotation = transform->FirstChildElement("rotation");
    if (rotation) {
        if (const char* x = rotation->Attribute("x")) instance.transform.rotX = std::stof(x);
        if (const char* y = rotation->Attribute("y")) instance.transform.rotY = std::stof(y);
        if (const char* z = rotation->Attribute("z")) instance.transform.rotZ = std::stof(z);
    }

    // Scale
    tinyxml2::XMLElement* scale = transform->FirstChildElement("scale");
    if (scale) {
        if (const char* x = scale->Attribute("x")) instance.transform.scaleX = std::stof(x);
        if (const char* y = scale->Attribute("y")) instance.transform.scaleY = std::stof(y);
        if (const char* z = scale->Attribute("z")) instance.transform.scaleZ = std::stof(z);
    }
}

void ConfigParser::parseCamera(void* node, SceneConfig& config) {
    tinyxml2::XMLElement* camera = static_cast<tinyxml2::XMLElement*>(node);

    config.camera.type = getElementText(camera, "type", "PERSPECTIVE");
    config.camera.fov = getElementFloat(camera, "fov", 60.0f);
    config.camera.nearPlane = getElementFloat(camera, "near", 0.1f);
    config.camera.farPlane = getElementFloat(camera, "far", 1000.0f);

    tinyxml2::XMLElement* position = camera->FirstChildElement("position");
    if (position) {
        if (const char* x = position->Attribute("x")) config.camera.posX = std::stof(x);
        if (const char* y = position->Attribute("y")) config.camera.posY = std::stof(y);
        if (const char* z = position->Attribute("z")) config.camera.posZ = std::stof(z);
    }

    tinyxml2::XMLElement* rotation = camera->FirstChildElement("rotation");
    if (rotation) {
        if (const char* x = rotation->Attribute("x")) config.camera.rotX = std::stof(x);
        if (const char* y = rotation->Attribute("y")) config.camera.rotY = std::stof(y);
        if (const char* z = rotation->Attribute("z")) config.camera.rotZ = std::stof(z);
    }

    compactLog("Camera", "type=" + config.camera.type +
               ", pos=[" + std::to_string(config.camera.posX) + "," + std::to_string(config.camera.posY) + "," + std::to_string(config.camera.posZ) + "]" +
               ", rot=[" + std::to_string(config.camera.rotX) + "," + std::to_string(config.camera.rotY) + "," + std::to_string(config.camera.rotZ) + "]" +
               ", fov=" + std::to_string(config.camera.fov));
}

std::string ConfigParser::getElementText(void* node, const char* path, const std::string& defaultValue) {
    tinyxml2::XMLElement* element = static_cast<tinyxml2::XMLElement*>(node);

    if (path == nullptr) {
        const char* text = element->GetText();
        return text ? std::string(text) : defaultValue;
    }

    tinyxml2::XMLElement* child = element->FirstChildElement(path);
    if (child) {
        const char* text = child->GetText();
        return text ? std::string(text) : defaultValue;
    }

    return defaultValue;
}

float ConfigParser::getElementFloat(void* node, const char* path, float defaultValue) {
    std::string strValue = getElementText(node, path, "");
    if (strValue.empty()) {
        return defaultValue;
    }

    try {
        return std::stof(strValue);
    } catch (...) {
        return defaultValue;
    }
}

std::string ConfigParser::getAttribute(void* element, const char* attrName, const std::string& defaultValue) {
    tinyxml2::XMLElement* elem = static_cast<tinyxml2::XMLElement*>(element);
    const char* attr = elem->Attribute(attrName);
    return attr ? std::string(attr) : defaultValue;
}

void ConfigParser::parseAssets(void* node, SceneConfig& config) {
    tinyxml2::XMLElement* assetsNode = static_cast<tinyxml2::XMLElement*>(node);

    // Parse mesh assets
    for (tinyxml2::XMLElement* mesh = assetsNode->FirstChildElement("mesh"); mesh; mesh = mesh->NextSiblingElement("mesh")) {
        AssetConfig asset;
        asset.id = getAttribute(mesh, "id", "");
        asset.src = getAttribute(mesh, "src", "");

        if (!asset.id.empty() && !asset.src.empty()) {
            config.assets.push_back(asset);
            compactLog("Asset", "type=mesh, id=" + asset.id + ", src=" + asset.src);
        }
    }

    // Parse texture assets
    for (tinyxml2::XMLElement* tex = assetsNode->FirstChildElement("texture"); tex; tex = tex->NextSiblingElement("texture")) {
        AssetConfig asset;
        asset.id = getAttribute(tex, "id", "");
        asset.src = getAttribute(tex, "src", "");

        if (!asset.id.empty() && !asset.src.empty()) {
            config.assets.push_back(asset);
            compactLog("Asset", "type=texture, id=" + asset.id + ", src=" + asset.src);
        }
    }
}

void ConfigParser::parseGeometry(void* node, ShaderInstanceConfig& instance) {
    tinyxml2::XMLElement* geometryNode = static_cast<tinyxml2::XMLElement*>(node);

    for (tinyxml2::XMLElement* mesh = geometryNode->FirstChildElement("mesh"); mesh; mesh = mesh->NextSiblingElement("mesh")) {
        GeometryConfig geom;
        geom.meshSrc = getAttribute(mesh, "src", "");

        if (!geom.meshSrc.empty()) {
            instance.geometry.push_back(geom);
        }
    }
}

bool ConfigParser::loadCoregfxConfig(const std::string& xmlPath) {
    ocean::info("Loading arctic.config.xml from: " + xmlPath);

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.c_str()) != tinyxml2::XML_SUCCESS) {
        ocean::error("Failed to load arctic.config.xml: " + xmlPath);
        return false;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("arctic-config");
    if (!root) {
        ocean::error("Invalid arctic.config.xml: missing <arctic-config> root element");
        return false;
    }

    // Parse <paths> section (newer syntax with direct tags)
    tinyxml2::XMLElement* pathsNode = root->FirstChildElement("paths");
    if (!pathsNode) {
        ocean::warn("No <paths> section found in arctic.config.xml");
        return false;
    }

    // Extract path values - support BOTH old and new XML syntax
    std::map<std::string, std::string> pathValues;

    // Try new syntax first (direct tags like <asset-root>)
    auto getPathValue = [&](const char* tagName, const char* configKey) {
        tinyxml2::XMLElement* elem = pathsNode->FirstChildElement(tagName);
        if (elem && elem->GetText()) {
            std::string value = elem->GetText();
            pathValues[configKey] = value;
            ocean::info("  Path loaded: " + std::string(configKey) + " = " + value);
            return true;
        }
        return false;
    };

    // Parse with new syntax
    getPathValue("asset-root", "ASSET_ROOT");
    getPathValue("shader-dir", "SHADER_DIR");
    getPathValue("log-dir", "LOG_DIR");
    getPathValue("temp-dir", "TEMP_DIR");

    // Fallback: Parse old syntax (<path name="..." value="..."/>)
    if (pathValues.empty()) {
        ocean::info("New syntax not found, trying old syntax with <path> attributes...");
        for (tinyxml2::XMLElement* pathElem = pathsNode->FirstChildElement("path");
             pathElem;
             pathElem = pathElem->NextSiblingElement("path")) {

            const char* name = pathElem->Attribute("name");
            const char* value = pathElem->Attribute("value");

            if (name && value) {
                pathValues[name] = value;
                ocean::info("  Path loaded (old syntax): " + std::string(name) + " = " + std::string(value));
            }
        }
    }

    if (pathValues.empty()) {
        ocean::warn("No paths found in arctic.config.xml (tried both old and new syntax)");
        return false;
    }

    // Initialize ConfigProvider with parsed paths
    ConfigProvider::initialize(pathValues);
    ocean::info("ConfigProvider initialized with " + std::to_string(pathValues.size()) + " paths");

    return true;
}

} // namespace datadriven
