#pragma once

#include "Config.hpp"
#include "GeometryFactory.hpp"

// Fix Windows macro conflicts with protobuf enums
#ifdef OPAQUE
#undef OPAQUE
#endif

#include <arctic_common.pb.h>
#include <arctic_authoring.pb.h>
#include <arctic_runtime.pb.h>
#include <arctic_network.pb.h>
#include <coregfx/core/ocean_core.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>

namespace datadriven {

/**
 * Unified uniform value storage (compatible with protobuf messages)
 */
struct UniformValue {
    enum class Type {
        Float,
        Vec3,
        Vec4
    };

    Type type;
    union {
        float f;
        glm::vec3 vec3;
        glm::vec4 vec4;
    } data;

    static UniformValue fromFloat(float value) {
        UniformValue uv;
        uv.type = Type::Float;
        uv.data.f = value;
        return uv;
    }

    static UniformValue fromVec3(float x, float y, float z) {
        UniformValue uv;
        uv.type = Type::Vec3;
        uv.data.vec3 = glm::vec3(x, y, z);
        return uv;
    }

    static UniformValue fromVec4(float x, float y, float z, float w) {
        UniformValue uv;
        uv.type = Type::Vec4;
        uv.data.vec4 = glm::vec4(x, y, z, w);
        return uv;
    }

    static UniformValue fromProto(const arctic::common::ArcticUniformValue& proto) {
        UniformValue uv;
        switch (proto.value_case()) {
            case arctic::common::ArcticUniformValue::kFloatValue:
                return fromFloat(proto.float_value());
            case arctic::common::ArcticUniformValue::kVec3Value:
                return fromVec3(proto.vec3_value().x(), proto.vec3_value().y(), proto.vec3_value().z());
            case arctic::common::ArcticUniformValue::kVec4Value:
                return fromVec4(proto.vec4_value().x(), proto.vec4_value().y(),
                               proto.vec4_value().z(), proto.vec4_value().w());
            default:
                return fromFloat(0.0f);
        }
    }
};


/**
 * Shader instance data with associated geometry
 */
struct ShaderInstance {
    std::string templateId;
    std::string instanceId;
    std::unordered_map<std::string, UniformValue> uniforms;
    std::unordered_map<std::string, std::string> textures;

    // Vulkan rendering resources
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    // Geometry (procedural or GLTF)
    GeometryData* geometry = nullptr;
    glm::mat4 transform = glm::mat4(1.0f); // Model transform matrix

    // Dirty flags for real-time updates
    bool uniformsDirty = false;  // Uniforms changed, need descriptor update
    bool texturesDirty = false;  // Textures changed, need descriptor update
};

/**
 * Data-Driven Shader Manager
 *
 * Manages shader instances loaded from XML and updated via gRPC.
 * Integrates with coregfx for Vulkan pipeline creation.
 */
class ShaderManager {
public:
    ShaderManager() = default;
    ~ShaderManager() = default;

    /**
     * Load shader templates from configuration
     */
    bool loadTemplates(const std::vector<ShaderTemplateConfig>& templates);

    /**
     * Initialize shader instances from configuration
     */
    bool initializeInstances(const std::vector<ShaderInstanceConfig>& instances);

    /**
     * Update shader instance from gRPC message
     */
    bool updateShaderInstance(const arctic::network::ShaderUpdate& update);

    /**
     * Get shader instance for rendering
     */
    ShaderInstance* getShaderInstance(const std::string& instanceId);

    /**
     * Get all shader instances (for rendering loop)
     */
    const std::unordered_map<std::string, std::shared_ptr<ShaderInstance>>& getAllInstances() const {
        return instances_;
    }

    /**
     * Attach geometry to a shader instance
     * @param instanceId Shader instance ID
     * @param geometry Geometry data (must remain valid for lifetime of instance)
     */
    bool attachGeometry(const std::string& instanceId, GeometryData* geometry);

    /**
     * Set transform for shader instance
     */
    bool setTransform(const std::string& instanceId, const glm::mat4& transform);

    /**
     * Create Vulkan buffers for geometry
     * @param vulkanDevice Vulkan device to create buffers on
     * @param geometry Geometry to create buffers for
     * @return true if successful
     */
    bool createGeometryBuffers(oceancore::VulkanDevice* vulkanDevice, GeometryData* geometry);

    /**
     * Set Vulkan device for pipeline and buffer creation
     */
    void setVulkanDevice(oceancore::VulkanDevice* device) { vulkanDevice_ = device; }

    /**
     * Apply real-time uniform update to GPU (from gRPC)
     * Updates shader instance uniforms and marks descriptor sets for refresh
     */
    bool applyUniformUpdate(const std::string& instanceId, const std::string& uniformName, const UniformValue& value);

    /**
     * Apply texture update to GPU (from gRPC)
     * Updates texture binding and marks descriptor sets for refresh
     */
    bool applyTextureUpdate(const std::string& instanceId, const std::string& binding, const std::string& texturePath);

    /**
     * Refresh descriptor sets for instances that have been updated
     * Call this before rendering to apply pending GPU updates
     */
    void refreshPendingDescriptorSets();

private:
    UniformValue parseUniformValue(const UniformConfig& config);
    bool createPipelineFromTemplate(ShaderInstance* instance);
    bool updateDescriptorSet(ShaderInstance* instance);

    std::unordered_map<std::string, std::shared_ptr<ShaderInstance>> instances_;
    std::unordered_map<std::string, std::string> templates_; // templateId -> JSON content

    // Vulkan device (set during initialization)
    oceancore::VulkanDevice* vulkanDevice_ = nullptr;
};

} // namespace datadriven
