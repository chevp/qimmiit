/**
 * Material Override Implementation
 */

#include "MaterialOverride.hpp"
#include <coregfx/core/ocean_log.hpp>

namespace datadriven {

void MaterialOverrideManager::setOverride(const std::string& instanceId, const std::string& propertyName, float value) {
    auto& override = overrides_[instanceId];

    if (propertyName == "metallic_factor" || propertyName == "metallicFactor") {
        override.metallicFactor = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else if (propertyName == "roughness_factor" || propertyName == "roughnessFactor") {
        override.roughnessFactor = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else if (propertyName == "emissive_strength" || propertyName == "emissiveStrength") {
        override.emissiveStrength = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else if (propertyName == "alpha_mask" || propertyName == "alphaMask") {
        override.alphaMask = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else if (propertyName == "alpha_mask_cutoff" || propertyName == "alphaMaskCutoff") {
        override.alphaMaskCutoff = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else if (propertyName == "normal_scale" || propertyName == "normalScale") {
        override.normalScale = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else if (propertyName == "occlusion_strength" || propertyName == "occlusionStrength") {
        override.occlusionStrength = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else if (propertyName == "transmission_factor" || propertyName == "transmissionFactor") {
        override.transmissionFactor = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else if (propertyName == "ior") {
        override.ior = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = " + std::to_string(value));
    }
    else {
        ocean::warn("Unknown material property: " + propertyName);
    }
}

void MaterialOverrideManager::setOverride(const std::string& instanceId, const std::string& propertyName, const glm::vec3& value) {
    auto& override = overrides_[instanceId];

    if (propertyName == "emissive_factor" || propertyName == "emissiveFactor") {
        override.emissiveFactor = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = [" +
                   std::to_string(value.x) + ", " +
                   std::to_string(value.y) + ", " +
                   std::to_string(value.z) + "]");
    }
    else {
        ocean::warn("Unknown vec3 material property: " + propertyName);
    }
}

void MaterialOverrideManager::setOverride(const std::string& instanceId, const std::string& propertyName, const glm::vec4& value) {
    auto& override = overrides_[instanceId];

    if (propertyName == "base_color_factor" || propertyName == "baseColorFactor") {
        override.baseColorFactor = value;
        ocean::info("Material override: " + instanceId + "." + propertyName + " = [" +
                   std::to_string(value.x) + ", " +
                   std::to_string(value.y) + ", " +
                   std::to_string(value.z) + ", " +
                   std::to_string(value.w) + "]");
    }
    else {
        ocean::warn("Unknown vec4 material property: " + propertyName);
    }
}

const MaterialOverride* MaterialOverrideManager::getOverride(const std::string& instanceId) const {
    auto it = overrides_.find(instanceId);
    if (it != overrides_.end() && it->second.hasOverrides()) {
        return &it->second;
    }
    return nullptr;
}

void MaterialOverrideManager::clearOverride(const std::string& instanceId) {
    auto it = overrides_.find(instanceId);
    if (it != overrides_.end()) {
        it->second.clear();
        ocean::info("Cleared material overrides for: " + instanceId);
    }
}

void MaterialOverrideManager::clearAll() {
    overrides_.clear();
    ocean::info("Cleared all material overrides");
}

bool MaterialOverrideManager::hasOverride(const std::string& instanceId) const {
    auto it = overrides_.find(instanceId);
    return it != overrides_.end() && it->second.hasOverrides();
}

} // namespace datadriven
