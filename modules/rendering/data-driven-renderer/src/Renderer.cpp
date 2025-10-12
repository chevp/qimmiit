/**
 * Data-Driven Renderer Implementation
 */

#include "Renderer.hpp"
#include "GeometryFactory.hpp"
#include <coregfx/core/ocean_log.hpp>
#include <coregfx/core/ocean_core.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>

namespace datadriven {

// Static instance pointer for window procedure
static Renderer* s_rendererInstance = nullptr;

// Compact logging helper
static void compactLog(const std::string& category, const std::string& data) {
    ocean::info("COMPACT: " + category + " - " + data);
}

Renderer::Renderer(const AppSettings& settings, int argc, char** argv)
    : settings_(settings)
    , pbrApp_(nullptr)
    , argc_(argc)
    , argv_(nullptr)
    , hInstance_(nullptr)
    , initialized_(false)
    , shouldClose_(false)
    , frameCount_(0)
    , lastFrameTime_(0.0f)
{
    ocean::info("Renderer constructor (argc=" + std::to_string(argc) + ")");

    // Copy argv strings to ensure they persist for OceanPbrApp's lifetime
    // CRITICAL: Reserve capacity BEFORE adding strings to prevent reallocation
    argStrings_.reserve(argc);
    argPointers_.reserve(argc + 1);  // +1 for safety

    // Copy all argv strings first
    for (int i = 0; i < argc; ++i) {
        ocean::info("argv[" + std::to_string(i) + "] = " + std::string(argv[i]));
        argStrings_.push_back(std::string(argv[i]));  // Use push_back, not emplace_back for clarity
    }

    // NOW create pointers to the stable strings
    // This MUST be done in a separate loop after all strings are added
    for (size_t i = 0; i < argStrings_.size(); ++i) {
        argPointers_.push_back(const_cast<char*>(argStrings_[i].c_str()));
    }

    argv_ = argPointers_.data();

    ocean::info("Renderer constructor - argc/argv setup complete");
    ocean::info("Testing argv_ stability:");
    for (int i = 0; i < argc_; ++i) {
        ocean::info("  argv_[" + std::to_string(i) + "] = " + std::string(argv_[i]));
    }

    s_rendererInstance = this;
}

Renderer::~Renderer() {
    ocean::info("Renderer destructor");
    shutdown();
    s_rendererInstance = nullptr;
}

bool Renderer::initialize(HINSTANCE hInstance) {
    ocean::info("Initializing Renderer...");
    hInstance_ = hInstance;

    // Initialize coregfx (OceanPbrApp) - simplified approach like elyrion
    if (!initializeCoregfx(hInstance)) {
        ocean::error("Failed to initialize coregfx");
        return false;
    }
    ocean::info("Coregfx initialized successfully");

    // Load the scene after OceanPbrApp is ready
    if (!loadScene(sceneConfig_)) {
        ocean::error("Failed to load scene");
        return false;
    }

    initialized_ = true;
    lastFrameTimePoint_ = std::chrono::steady_clock::now();

    ocean::info("Renderer initialization complete");
    return true;
}

// Window creation is now handled by OceanPbrApp::init()
// This method is no longer needed since we follow the elyrion pattern

bool Renderer::initializeCoregfx(HINSTANCE hInstance) {
    ocean::info("Initializing OceanPbrApp...");

    try {
        // Create OceanPbrApp using stored argc_/argv_ (passed from WinMain)
        // Note: Global __argc/__argv only contain exe path in WinMain, not lpCmdLine args
        ocean::info("Creating OceanPbrApp with argc=" + std::to_string(argc_) + "...");
        pbrApp_ = new coregfx::OceanPbrApp(argc_, argv_);

        if (!pbrApp_) {
            ocean::error("Failed to create OceanPbrApp");
            return false;
        }
        ocean::info("OceanPbrApp instance created successfully");

        // Use default settings like elyrion does
        auto settings = coregfx::OceanPbrApp::buildDefaultSettings();
        settings.validation = settings_.studioMode;
        settings.vsync = true;

        ocean::info("=== STARTING VULKAN INITIALIZATION ===");
        ocean::info("Calling pbrApp_->init() with default settings...");
        ocean::info("Settings: validation=" + std::to_string(settings.validation) + ", vsync=" + std::to_string(settings.vsync));

        try {
            ocean::info("About to call pbrApp_->init()...");
            pbrApp_->init(&settings, hInstance, Renderer::windowProc);
            ocean::info("pbrApp_->init() completed successfully");
            ocean::info("=== VULKAN INITIALIZATION COMPLETE ===");
        } catch (const std::exception& e) {
            ocean::error("EXCEPTION in pbrApp_->init(): " + std::string(e.what()));
            // CRITICAL: Don't delete pbrApp_ - its destructor crashes when init() fails
            // Just set to nullptr to prevent shutdown() from trying to delete it
            // This is a small memory leak, but better than crashing
            ocean::warn("Leaking OceanPbrApp memory to avoid destructor crash (init failed)");
            pbrApp_ = nullptr;
            throw;
        } catch (...) {
            ocean::error("UNKNOWN EXCEPTION in pbrApp_->init()");
            // CRITICAL: Don't delete pbrApp_ - its destructor crashes when init() fails
            ocean::warn("Leaking OceanPbrApp memory to avoid destructor crash (init failed)");
            pbrApp_ = nullptr;
            throw;
        }

        // OceanPbrApp creates and manages the window internally
        ocean::info("OceanPbrApp initialized successfully (window created by OceanPbrApp)");
        return true;

    } catch (const std::exception& e) {
        ocean::error("Exception during coregfx initialization: " + std::string(e.what()));
        // pbrApp_ is already deleted in the inner catch block if init() failed
        return false;
    }
}

bool Renderer::loadScene(const SceneConfig& config) {
    ocean::info("Loading scene: " + config.name);
    sceneConfig_ = config;

    // Load shader templates and instances
    if (!loadShaderTemplatesAndInstances(config)) {
        ocean::error("Failed to load shader templates and instances");
        return false;
    }

    // Set camera from config
    updateCamera(
        glm::vec3(config.camera.posX, config.camera.posY, config.camera.posZ),
        glm::vec3(0.0f, 0.0f, 0.0f)  // TODO: Parse rotation from config
    );

    ocean::info("Scene loaded successfully: " + config.name);
    return true;
}

bool Renderer::loadShaderTemplatesAndInstances(const SceneConfig& config) {
    ocean::info("Loading shader templates (" + std::to_string(config.shaderTemplates.size()) + ")...");

    if (!shaderManager_.loadTemplates(config.shaderTemplates)) {
        ocean::warn("No shader templates loaded");
    }

    ocean::info("Initializing shader instances (" + std::to_string(config.shaderInstances.size()) + ")...");

    if (!shaderManager_.initializeInstances(config.shaderInstances)) {
        ocean::warn("No shader instances initialized");
    }

    // Load assets (GLTF models) - following elyrion pattern
    ocean::info("Loading assets (" + std::to_string(config.assets.size()) + ")...");

    // Register assets in resolver
    for (const auto& asset : config.assets) {
        assetResolver_.registerAsset(asset.id, asset.src);
        ocean::info("Registered asset: " + asset.id + " -> " + asset.src);
    }

    // Step 1: Deduplicate and load unique GLTF models
    std::unordered_map<std::string, uint32_t> meshSrcToGltfId; // Track loaded models to avoid duplicates
    uint32_t gltfId = 1; // Start from 1 like elyrion

    for (const auto& instanceConfig : config.shaderInstances) {
        for (const auto& geom : instanceConfig.geometry) {
            if (!geom.meshSrc.empty()) {
                // Check if already loaded
                if (meshSrcToGltfId.find(geom.meshSrc) != meshSrcToGltfId.end()) {
                    compactLog("GLTF Reuse", "instance=" + instanceConfig.id + ", meshSrc=" + geom.meshSrc + ", gltfId=" + std::to_string(meshSrcToGltfId[geom.meshSrc]));
                    continue; // Skip - already loaded
                }

                ocean::info("Loading GLTF for instance " + instanceConfig.id + ": " + geom.meshSrc);
                std::string resolvedPath = assetResolver_.resolveAssetUri(geom.meshSrc);

                if (assetLoader_.loadGltfModel(std::to_string(gltfId), resolvedPath)) {
                    const tinygltf::Model* model = assetLoader_.getGltfModel(std::to_string(gltfId));
                    if (model) {
                        gltfModelContext_.emplace_back(gltfId, GltfEngine::GltfService::GLTF_READY, *model);
                        meshSrcToGltfId[geom.meshSrc] = gltfId; // Track this mesh source
                        compactLog("GLTF Unique", "gltfId=" + std::to_string(gltfId) + ", meshSrc=" + geom.meshSrc);
                        gltfId++;
                    }
                }
            }
        }
    }

    // Step 2: Insert all unique GLTF models into OceanPbrApp
    if (!gltfModelContext_.empty() && pbrApp_) {
        ocean::info("Inserting " + std::to_string(gltfModelContext_.size()) + " unique GLTF models into renderer...");
        pbrApp_->insertGltf3(&gltfModelContext_);
        ocean::info("GLTF models inserted successfully");
    }

    // Step 3: Create nodes with transforms for each shader instance (like elyrion)
    uint32_t nodeId = 1;
    for (const auto& instanceConfig : config.shaderInstances) {
        for (const auto& geom : instanceConfig.geometry) {
            if (!geom.meshSrc.empty()) {
                // Find the GLTF ID for this mesh source
                auto it = meshSrcToGltfId.find(geom.meshSrc);
                if (it != meshSrcToGltfId.end()) {
                    uint32_t gltfModelId = it->second;

                    // Extract transform
                    glm::vec3 position(instanceConfig.transform.posX, instanceConfig.transform.posY, instanceConfig.transform.posZ);
                    glm::vec3 rotation(instanceConfig.transform.rotX, instanceConfig.transform.rotY, instanceConfig.transform.rotZ);

                    // Create node with transform
                    pbrApp_->insertNode(nodeId, gltfModelId, position, rotation);

                    compactLog("Node Created", "nodeId=" + std::to_string(nodeId) +
                               ", gltfId=" + std::to_string(gltfModelId) +
                               ", instanceId=" + instanceConfig.id +
                               ", pos=[" + std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z) + "]");

                    nodeId++;
                }
            }
        }
    }

    return true;
}

void Renderer::renderShaderInstances(VkCommandBuffer commandBuffer) {
    // Get all shader instances from manager
    const auto& instances = shaderManager_.getAllInstances();

    ocean::info("Rendering " + std::to_string(instances.size()) + " shader instances");

    for (const auto& [instanceId, instance] : instances) {
        if (!instance->geometry) {
            // Skipping instance without geometry (silent)
            continue;
        }

        // TODO: Bind pipeline
        // vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, instance->pipeline);

        // TODO: Bind descriptor sets (uniforms, textures)
        // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ...);

        // Bind vertex buffer
        VkBuffer vertexBuffers[] = { instance->geometry->vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        // Bind index buffer
        vkCmdBindIndexBuffer(commandBuffer, instance->geometry->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        // TODO: Push constants for transform matrix
        // vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &instance->transform);

        // Draw indexed
        uint32_t indexCount = static_cast<uint32_t>(instance->geometry->indices.size());
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

        ocean::info("Drew instance " + instanceId + " (" + std::to_string(indexCount) + " indices)");
    }
}

void Renderer::renderFrame() {
    if (!initialized_ || !pbrApp_) {
        return;
    }

    // Calculate frame time
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameTimePoint_);
    lastFrameTime_ = static_cast<float>(duration.count());
    lastFrameTimePoint_ = now;

    // Phase 3: Refresh descriptor sets for shader instances with pending updates (from gRPC)
    shaderManager_.refreshPendingDescriptorSets();

    // Render frame using OceanPbrApp (handles Vulkan setup, command buffer recording)
    try {
        pbrApp_->render2();

        // TODO: Integrate renderShaderInstances into OceanPbrApp's command buffer
        // This requires either:
        // 1. Extending OceanPbrApp::render2() to accept a callback for custom rendering
        // 2. Getting access to the command buffer and rendering after PBR content
        // 3. Creating a completely custom render pass for data-driven instances

        frameCount_++;
    } catch (const std::exception& e) {
        ocean::error("Exception during render: " + std::string(e.what()));
        shouldClose_ = true;
    }
}

bool Renderer::isInitialized() const {
    return initialized_ && pbrApp_ != nullptr && pbrApp_->isPrepared();
}

bool Renderer::shouldClose() const {
    return shouldClose_;
}

void Renderer::shutdown() {
    ocean::info("Shutting down Renderer...");

    if (pbrApp_) {
        // TODO: Proper cleanup of OceanPbrApp
        // For now, just delete (may need more sophisticated cleanup)
        delete pbrApp_;
        pbrApp_ = nullptr;
    }

    // Window is managed by OceanPbrApp - no need to destroy it here

    initialized_ = false;
    ocean::info("Renderer shutdown complete");
}

void Renderer::updateShaderInstance(const arctic::network::ShaderUpdate& update) {
    ocean::info("Received gRPC shader update for instance: " + update.shader_instance_id() +
               " (uniforms: " + std::to_string(update.uniform_updates_size()) +
               ", textures: " + std::to_string(update.texture_updates_size()) + ")");

    // Apply uniform updates to MaterialOverrideManager (Hybrid approach - override GLTF materials)
    for (const auto& [name, protoValue] : update.uniform_updates()) {
        UniformValue value = UniformValue::fromProto(protoValue);

        // Apply to material override system for OceanPbrApp
        if (protoValue.has_float_value()) {
            materialOverrides_.setOverride(update.shader_instance_id(), name, protoValue.float_value());
        }
        else if (protoValue.has_vec3_value()) {
            const auto& v = protoValue.vec3_value();
            materialOverrides_.setOverride(update.shader_instance_id(), name, glm::vec3(v.x(), v.y(), v.z()));
        }
        else if (protoValue.has_vec4_value()) {
            const auto& v = protoValue.vec4_value();
            materialOverrides_.setOverride(update.shader_instance_id(), name, glm::vec4(v.x(), v.y(), v.z(), v.w()));
        }

        // Also apply to ShaderManager for custom pipeline support (future)
        shaderManager_.applyUniformUpdate(update.shader_instance_id(), name, value);
    }

    // Apply texture updates
    for (const auto& [binding, texturePath] : update.texture_updates()) {
        shaderManager_.applyTextureUpdate(update.shader_instance_id(), binding, texturePath);
    }

    ocean::info("Material overrides applied - will be used in next frame render");
}

void Renderer::updateCamera(const glm::vec3& position, const glm::vec3& rotation) {
    ocean::info("Updating camera position: [" +
        std::to_string(position.x) + ", " +
        std::to_string(position.y) + ", " +
        std::to_string(position.z) + "]");

    if (pbrApp_) {
        pbrApp_->updateCamera(position, rotation);
    }
}

LRESULT CALLBACK Renderer::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (s_rendererInstance) {
        // Forward ALL messages to OceanPbrApp's handleMessages for ImGui and camera input
        if (s_rendererInstance->pbrApp_) {
            s_rendererInstance->pbrApp_->handleMessages(hwnd, msg, wParam, lParam);
        }

        switch (msg) {
        case WM_CLOSE:
            ocean::info("WM_CLOSE received");
            s_rendererInstance->shouldClose_ = true;
            return 0;

        case WM_DESTROY:
            ocean::info("WM_DESTROY received");
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            // Handle window resize
            // TODO: Notify OceanPbrApp of resize
            break;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

} // namespace datadriven
