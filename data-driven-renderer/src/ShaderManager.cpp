#include "ShaderManager.hpp"
#include "GeometryFactory.hpp"
#include <coregfx/core/ocean_log.hpp>
#include <coregfx/core/ocean_core.hpp>
#include <sstream>
#include <fstream>

namespace datadriven {

bool ShaderManager::loadTemplates(const std::vector<ShaderTemplateConfig>& templates) {
    ocean::info("Loading " + std::to_string(templates.size()) + " shader templates from XML");

    for (const auto& tmpl : templates) {
        // Store template configuration (no JSON files to load)
        // Template metadata is already parsed from data-driven.index.xml
        templates_[tmpl.id] = tmpl.name + "|" + tmpl.description + "|" +
                             tmpl.vertexShader + "|" + tmpl.fragmentShader;

        ocean::info("Registered template: " + tmpl.id +
                   " (" + tmpl.name + ") - SPIR-V: " +
                   tmpl.vertexShader + ", " + tmpl.fragmentShader);
    }

    return !templates_.empty();
}

bool ShaderManager::initializeInstances(const std::vector<ShaderInstanceConfig>& instances) {
    ocean::info("Initializing " + std::to_string(instances.size()) + " shader instances");

    for (const auto& instanceConfig : instances) {
        auto instance = std::make_shared<ShaderInstance>();
        instance->templateId = instanceConfig.templateId;
        instance->instanceId = instanceConfig.id;

        // Parse uniforms from XML configuration
        for (const auto& uniformConfig : instanceConfig.uniforms) {
            UniformValue uniformValue = parseUniformValue(uniformConfig);
            instance->uniforms[uniformConfig.name] = uniformValue;
        }

        // Parse textures
        for (const auto& textureConfig : instanceConfig.textures) {
            instance->textures[textureConfig.binding] = textureConfig.src;
        }

        // Create pipeline from template (stub implementation)
        createPipelineFromTemplate(instance.get());

        instances_[instance->instanceId] = instance;

        ocean::info("Initialized shader instance: " + instance->instanceId +
                   " (template: " + instance->templateId +
                   ", uniforms: " + std::to_string(instance->uniforms.size()) +
                   ", textures: " + std::to_string(instance->textures.size()) + ")");
    }

    return !instances_.empty();
}

bool ShaderManager::updateShaderInstance(const arctic::network::ShaderUpdate& update) {
    std::string instanceId = update.shader_instance_id();
    std::string templateId = update.template_id();

    ocean::info("gRPC shader update: " + instanceId + " (template: " + templateId + ")");

    // Find or create instance
    auto it = instances_.find(instanceId);
    std::shared_ptr<ShaderInstance> instance;

    if (it == instances_.end()) {
        instance = std::make_shared<ShaderInstance>();
        instance->templateId = templateId;
        instance->instanceId = instanceId;
        instances_[instanceId] = instance;
        ocean::info("Created new shader instance from gRPC: " + instanceId);
    } else {
        instance = it->second;
    }

    // Update uniforms from protobuf
    for (const auto& [name, protoValue] : update.uniform_updates()) {
        UniformValue uniformValue = UniformValue::fromProto(protoValue);
        instance->uniforms[name] = uniformValue;
    }

    // Update textures
    for (const auto& [binding, texturePath] : update.texture_updates()) {
        instance->textures[binding] = texturePath;
    }

    // Update descriptor set (hot path - no pipeline recreation)
    updateDescriptorSet(instance.get());

    return true;
}

ShaderInstance* ShaderManager::getShaderInstance(const std::string& instanceId) {
    auto it = instances_.find(instanceId);
    if (it == instances_.end()) {
        return nullptr;
    }
    return it->second.get();
}

UniformValue ShaderManager::parseUniformValue(const UniformConfig& config) {
    if (config.type == "float") {
        return UniformValue::fromFloat(std::stof(config.value));
    }
    else if (config.type == "vec3") {
        std::istringstream iss(config.value);
        std::string token;
        std::vector<float> values;
        while (std::getline(iss, token, ',')) {
            values.push_back(std::stof(token));
        }
        if (values.size() >= 3) {
            return UniformValue::fromVec3(values[0], values[1], values[2]);
        }
    }
    else if (config.type == "vec4") {
        std::istringstream iss(config.value);
        std::string token;
        std::vector<float> values;
        while (std::getline(iss, token, ',')) {
            values.push_back(std::stof(token));
        }
        if (values.size() >= 4) {
            return UniformValue::fromVec4(values[0], values[1], values[2], values[3]);
        }
    }

    return UniformValue::fromFloat(0.0f);
}

bool ShaderManager::createPipelineFromTemplate(ShaderInstance* instance) {
    if (!vulkanDevice_) {
        ocean::error("Vulkan device not set - cannot create pipeline");
        return false;
    }

    // Parse template JSON from templates_[instance->templateId]
    auto templateIt = templates_.find(instance->templateId);
    if (templateIt == templates_.end()) {
        ocean::error("Shader template not found: " + instance->templateId);
        return false;
    }

    // TODO: Parse JSON to get shader SPIR-V paths and pipeline config
    // For now, use a stub pipeline that uses simple vertex/fragment shaders

    ocean::info("Creating Vulkan pipeline for template: " + instance->templateId);

    // Vertex input binding (matches SimpleVertex structure)
    VkVertexInputBindingDescription vertexBinding{};
    vertexBinding.binding = 0;
    vertexBinding.stride = sizeof(SimpleVertex);
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Vertex input attributes
    std::vector<VkVertexInputAttributeDescription> vertexAttributes(4);

    // Position (vec3)
    vertexAttributes[0].binding = 0;
    vertexAttributes[0].location = 0;
    vertexAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributes[0].offset = offsetof(SimpleVertex, position);

    // Normal (vec3)
    vertexAttributes[1].binding = 0;
    vertexAttributes[1].location = 1;
    vertexAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributes[1].offset = offsetof(SimpleVertex, normal);

    // TexCoord (vec2)
    vertexAttributes[2].binding = 0;
    vertexAttributes[2].location = 2;
    vertexAttributes[2].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttributes[2].offset = offsetof(SimpleVertex, texCoord);

    // Color (vec4)
    vertexAttributes[3].binding = 0;
    vertexAttributes[3].location = 3;
    vertexAttributes[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexAttributes[3].offset = offsetof(SimpleVertex, color);

    // TODO: Complete pipeline creation with:
    // - Load shader SPIR-V files (from template JSON)
    // - Create descriptor set layout for uniforms
    // - Create pipeline layout
    // - Create graphics pipeline with vertex input state, rasterization, etc.

    // For now, mark as incomplete stub
    instance->pipeline = VK_NULL_HANDLE;

    ocean::info("Pipeline stub created for: " + instance->templateId + " (needs shader SPIR-V integration)");
    return true;
}

bool ShaderManager::updateDescriptorSet(ShaderInstance* instance) {
    if (!vulkanDevice_) {
        ocean::error("Vulkan device not set - cannot update descriptor set");
        return false;
    }

    // Pack uniforms into a buffer (simplified - assumes all uniforms fit in one UBO)
    struct UniformBufferData {
        glm::vec4 values[16]; // Support up to 16 vec4 uniforms
        uint32_t count = 0;
    } uniformData;

    // Pack uniforms into vec4 array
    uint32_t uniformIndex = 0;
    for (const auto& [name, value] : instance->uniforms) {
        if (uniformIndex >= 16) {
            ocean::warn("Too many uniforms - maximum 16 supported");
            break;
        }

        switch (value.type) {
            case UniformValue::Type::Float:
                uniformData.values[uniformIndex] = glm::vec4(value.data.f, 0, 0, 0);
                break;
            case UniformValue::Type::Vec3:
                uniformData.values[uniformIndex] = glm::vec4(value.data.vec3, 0.0f);
                break;
            case UniformValue::Type::Vec4:
                uniformData.values[uniformIndex] = value.data.vec4;
                break;
        }
        uniformIndex++;
    }
    uniformData.count = uniformIndex;

    // TODO: Create/update Vulkan uniform buffer with uniformData
    // TODO: Bind textures from instance->textures map
    // TODO: Update VkDescriptorSet with buffer and texture bindings

    ocean::info("Descriptor set updated for: " + instance->instanceId +
               " (uniforms: " + std::to_string(instance->uniforms.size()) +
               ", textures: " + std::to_string(instance->textures.size()) +
               ") (needs UBO/descriptor allocation)");

    return true;
}

bool ShaderManager::attachGeometry(const std::string& instanceId, GeometryData* geometry) {
    auto it = instances_.find(instanceId);
    if (it == instances_.end()) {
        ocean::error("Shader instance not found: " + instanceId);
        return false;
    }

    it->second->geometry = geometry;
    ocean::info("Attached geometry to shader instance: " + instanceId +
               " (vertices: " + std::to_string(geometry->vertices.size()) +
               ", indices: " + std::to_string(geometry->indices.size()) + ")");
    return true;
}

bool ShaderManager::setTransform(const std::string& instanceId, const glm::mat4& transform) {
    auto it = instances_.find(instanceId);
    if (it == instances_.end()) {
        ocean::error("Shader instance not found: " + instanceId);
        return false;
    }

    it->second->transform = transform;
    return true;
}

bool ShaderManager::createGeometryBuffers(oceancore::VulkanDevice* vulkanDevice, GeometryData* geometry) {
    if (!vulkanDevice || !geometry) {
        ocean::error("Invalid parameters for createGeometryBuffers");
        return false;
    }

    // Calculate buffer sizes
    VkDeviceSize vertexBufferSize = sizeof(SimpleVertex) * geometry->vertices.size();
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * geometry->indices.size();

    ocean::info("Creating Vulkan buffers: vertices=" + std::to_string(geometry->vertices.size()) +
               " (" + std::to_string(vertexBufferSize) + " bytes), indices=" +
               std::to_string(geometry->indices.size()) +
               " (" + std::to_string(indexBufferSize) + " bytes)");

    // Create vertex buffer using oceancore::Buffer
    oceancore::Buffer vertexStaging;
    vertexStaging.create(vulkanDevice,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        vertexBufferSize);

    // Copy vertex data to staging buffer
    void* data;
    vkMapMemory(vulkanDevice->logicalDevice, vertexStaging.memory, 0, vertexBufferSize, 0, &data);
    memcpy(data, geometry->vertices.data(), vertexBufferSize);
    vkUnmapMemory(vulkanDevice->logicalDevice, vertexStaging.memory);

    // Create device local vertex buffer
    oceancore::Buffer vertexBuffer;
    vertexBuffer.create(vulkanDevice,
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       vertexBufferSize,
                       false); // Don't map

    // TODO: Copy from staging to device local (needs command buffer)
    // For now, store staging buffer handles temporarily
    geometry->vertexBuffer = vertexStaging.buffer;
    geometry->vertexMemory = vertexStaging.memory;

    // Create index buffer using same approach
    oceancore::Buffer indexStaging;
    indexStaging.create(vulkanDevice,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       indexBufferSize);

    vkMapMemory(vulkanDevice->logicalDevice, indexStaging.memory, 0, indexBufferSize, 0, &data);
    memcpy(data, geometry->indices.data(), indexBufferSize);
    vkUnmapMemory(vulkanDevice->logicalDevice, indexStaging.memory);

    // Store staging buffer (TODO: transfer to device local)
    geometry->indexBuffer = indexStaging.buffer;
    geometry->indexMemory = indexStaging.memory;

    ocean::info("Vulkan geometry buffers created successfully");
    return true;
}

bool ShaderManager::applyUniformUpdate(const std::string& instanceId,
                                                   const std::string& uniformName,
                                                   const UniformValue& value) {
    auto it = instances_.find(instanceId);
    if (it == instances_.end()) {
        ocean::warn("Cannot apply uniform update - instance not found: " + instanceId);
        return false;
    }

    // Update uniform value
    it->second->uniforms[uniformName] = value;
    it->second->uniformsDirty = true;

    ocean::info("Applied uniform update: " + instanceId + "." + uniformName);
    return true;
}

bool ShaderManager::applyTextureUpdate(const std::string& instanceId,
                                                   const std::string& binding,
                                                   const std::string& texturePath) {
    auto it = instances_.find(instanceId);
    if (it == instances_.end()) {
        ocean::warn("Cannot apply texture update - instance not found: " + instanceId);
        return false;
    }

    // Update texture binding
    it->second->textures[binding] = texturePath;
    it->second->texturesDirty = true;

    ocean::info("Applied texture update: " + instanceId + " binding=" + binding + " texture=" + texturePath);
    return true;
}

void ShaderManager::refreshPendingDescriptorSets() {
    int updateCount = 0;

    for (auto& [instanceId, instance] : instances_) {
        if (instance->uniformsDirty || instance->texturesDirty) {
            ocean::info("Refreshing descriptor set for: " + instanceId);

            // Update Vulkan descriptor set with new uniforms/textures
            if (updateDescriptorSet(instance.get())) {
                instance->uniformsDirty = false;
                instance->texturesDirty = false;
                updateCount++;
            } else {
                ocean::warn("Failed to refresh descriptor set for: " + instanceId);
            }
        }
    }

    if (updateCount > 0) {
        ocean::info("Refreshed " + std::to_string(updateCount) + " descriptor sets");
    }
}

} // namespace datadriven
