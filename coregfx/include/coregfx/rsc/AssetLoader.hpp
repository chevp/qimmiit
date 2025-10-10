/**
 * Asset Loader for Data-Driven Renderer
 *
 * Loads GLTF models and textures from asset:// URIs referenced in
 * shader instances from arctic.xml
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <tinygltf/tiny_gltf.h>

namespace coregfx {

/**
 * Resolves asset:// URIs to file system paths
 * Example: asset://12345 -> models/metal_crate.gltf
 */
class AssetResolver {
public:
    /**
     * Register an asset ID with its file path
     */
    void registerAsset(const std::string& assetId, const std::string& filePath);

    /**
     * Resolve asset:// URI to file path
     * @param uri Format: "asset://12345" or just "12345"
     * @return File path or empty string if not found
     */
    std::string resolveAssetUri(const std::string& uri) const;

    /**
     * Check if asset exists
     */
    bool hasAsset(const std::string& assetId) const;

private:
    std::unordered_map<std::string, std::string> assets_;
};

/**
 * Loads GLTF models from file system
 * Integrates with coregfx's GltfLoader
 */
class AssetLoader {
public:
    AssetLoader();
    ~AssetLoader();

    /**
     * Load a GLTF model from file path
     * @param assetId Unique identifier for the model
     * @param filePath Path to .gltf or .glb file
     * @return true if loaded successfully
     */
    bool loadGltfModel(const std::string& assetId, const std::string& filePath);

    /**
     * Get loaded GLTF model
     * @param assetId Model identifier
     * @return Pointer to tinygltf::Model or nullptr if not found
     */
    const tinygltf::Model* getGltfModel(const std::string& assetId) const;

    /**
     * Get all loaded models
     */
    const std::unordered_map<std::string, std::unique_ptr<tinygltf::Model>>& getAllModels() const {
        return models_;
    }

    /**
     * Load texture from file path
     * @param binding Texture binding name (e.g., "base_color_texture")
     * @param filePath Path to texture file
     * @return true if loaded successfully
     */
    bool loadTexture(const std::string& binding, const std::string& filePath);

private:
    std::unordered_map<std::string, std::unique_ptr<tinygltf::Model>> models_;
    std::unordered_map<std::string, std::string> textures_;
};

} // namespace coregfx
