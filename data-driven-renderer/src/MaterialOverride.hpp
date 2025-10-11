/**
 * Material Override System
 *
 * Provides gRPC-driven material property overrides for GLTF models
 * rendered by OceanPbrApp.
 *
 * Flow:
 * 1. GLTF models load with embedded materials (defaults)
 * 2. Server sends gRPC ShaderUpdate messages
 * 3. MaterialOverrideManager stores override values
 * 4. Before rendering, apply overrides to PushConstBlockMaterial
 * 5. OceanPbrApp renders with modified materials
 */

#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <glm/glm.hpp>

namespace datadriven {

/**
 * Material property overrides for a single shader instance
 */
struct MaterialOverride {
    // PBR Metallic-Roughness properties
    std::optional<glm::vec4> baseColorFactor;
    std::optional<float> metallicFactor;
    std::optional<float> roughnessFactor;

    // Emissive properties
    std::optional<glm::vec3> emissiveFactor;
    std::optional<float> emissiveStrength;

    // Advanced properties
    std::optional<float> alphaMask;
    std::optional<float> alphaMaskCutoff;
    std::optional<float> normalScale;
    std::optional<float> occlusionStrength;

    // Transmission/Glass properties (KHR_materials_transmission)
    std::optional<float> transmissionFactor;
    std::optional<float> ior;  // Index of refraction

    /**
     * Check if any overrides are set
     */
    bool hasOverrides() const {
        return baseColorFactor.has_value() ||
               metallicFactor.has_value() ||
               roughnessFactor.has_value() ||
               emissiveFactor.has_value() ||
               emissiveStrength.has_value() ||
               alphaMask.has_value() ||
               alphaMaskCutoff.has_value() ||
               normalScale.has_value() ||
               occlusionStrength.has_value() ||
               transmissionFactor.has_value() ||
               ior.has_value();
    }

    /**
     * Clear all overrides (revert to GLTF defaults)
     */
    void clear() {
        baseColorFactor.reset();
        metallicFactor.reset();
        roughnessFactor.reset();
        emissiveFactor.reset();
        emissiveStrength.reset();
        alphaMask.reset();
        alphaMaskCutoff.reset();
        normalScale.reset();
        occlusionStrength.reset();
        transmissionFactor.reset();
        ior.reset();
    }
};

/**
 * Manages material overrides for all shader instances
 */
class MaterialOverrideManager {
public:
    /**
     * Set material property override for a shader instance
     */
    void setOverride(const std::string& instanceId, const std::string& propertyName, float value);
    void setOverride(const std::string& instanceId, const std::string& propertyName, const glm::vec3& value);
    void setOverride(const std::string& instanceId, const std::string& propertyName, const glm::vec4& value);

    /**
     * Get material overrides for a shader instance
     */
    const MaterialOverride* getOverride(const std::string& instanceId) const;

    /**
     * Remove all overrides for a shader instance
     */
    void clearOverride(const std::string& instanceId);

    /**
     * Remove all overrides
     */
    void clearAll();

    /**
     * Check if instance has any overrides
     */
    bool hasOverride(const std::string& instanceId) const;

private:
    // Map: instance ID -> material overrides
    std::unordered_map<std::string, MaterialOverride> overrides_;
};

} // namespace datadriven
