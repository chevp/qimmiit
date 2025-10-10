/**
 * Asset Loader Implementation
 */

#include <coregfx/rsc/AssetLoader.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <coregfx/core/ConfigProvider.hpp>
#include <algorithm>
#include <filesystem>

namespace coregfx {

// Compact XML-style logging helper
static void compactLog(const std::string& category, const std::string& data) {
    ocean::info("COMPACT: " + category + " - " + data);
}

// AssetResolver implementation

void AssetResolver::registerAsset(const std::string& assetId, const std::string& filePath) {
    assets_[assetId] = filePath;
    ocean::info("Registered asset: " + assetId + " -> " + filePath);
}

std::string AssetResolver::resolveAssetUri(const std::string& uri) const {
    std::string path = uri;

    // Handle asset:// protocol - resolve to ASSET_ROOT
    if (path.rfind("asset://", 0) == 0) {
        path = path.substr(8); // Remove "asset://"

        // Get ASSET_ROOT from config (fallback to hardcoded path)
        std::string assetRoot = "assets/vkpbr5";

        // Remove trailing slash if present
        if (!assetRoot.empty() && (assetRoot.back() == '/' || assetRoot.back() == '\\')) {
            assetRoot = assetRoot.substr(0, assetRoot.length() - 1);
        }

        path = assetRoot + "/" + path;

        ocean::info("Asset resolved: asset:// -> " + path);
    }
    // Check if it's a registered asset ID
    else if (assets_.find(uri) != assets_.end()) {
        path = assets_.at(uri);
        ocean::info("Asset resolved from registry: " + uri + " -> " + path);
    }

    // Convert forward slashes to backslashes for Windows
    std::replace(path.begin(), path.end(), '/', '\\');

    return path;
}

bool AssetResolver::hasAsset(const std::string& assetId) const {
    return assets_.find(assetId) != assets_.end();
}

// AssetLoader implementation

AssetLoader::AssetLoader() {
    ocean::info("AssetLoader initialized");
}

AssetLoader::~AssetLoader() {
    ocean::info("AssetLoader cleanup");
}

bool AssetLoader::loadGltfModel(const std::string& assetId, const std::string& filePath) {
    ocean::info("Loading GLTF model: " + assetId + " from " + filePath);

    // Check if already loaded
    if (models_.find(assetId) != models_.end()) {
        ocean::warn("GLTF model already loaded: " + assetId);
        return true;
    }

    // Create new model
    auto model = std::make_unique<tinygltf::Model>();

    // Load using TinyGLTF
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool success = false;

    // Determine file type by extension
    std::string ext = filePath.substr(filePath.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "gltf") {
        success = loader.LoadASCIIFromFile(model.get(), &err, &warn, filePath);
    } else if (ext == "glb") {
        success = loader.LoadBinaryFromFile(model.get(), &err, &warn, filePath);
    } else {
        ocean::error("Unknown GLTF file extension: " + ext);
        return false;
    }

    if (!warn.empty()) {
        ocean::warn("GLTF loader warning: " + warn);
    }

    if (!success || !err.empty()) {
        ocean::error("Failed to load GLTF model: " + filePath + " - " + err);
        return false;
    }

    compactLog("GLTF Load", "id=" + assetId +
               ", meshes=" + std::to_string(model->meshes.size()) +
               ", materials=" + std::to_string(model->materials.size()) +
               ", textures=" + std::to_string(model->textures.size()) +
               ", path=" + filePath);

    // Store model
    models_[assetId] = std::move(model);

    return true;
}

const tinygltf::Model* AssetLoader::getGltfModel(const std::string& assetId) const {
    auto it = models_.find(assetId);
    if (it != models_.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool AssetLoader::loadTexture(const std::string& binding, const std::string& filePath) {
    ocean::info("Loading texture: " + binding + " from " + filePath);

    // For now, just store the path - actual Vulkan texture loading will be done later
    textures_[binding] = filePath;

    // TODO: Load texture with Vulkan and create VkImage

    return true;
}

} // namespace coregfx
