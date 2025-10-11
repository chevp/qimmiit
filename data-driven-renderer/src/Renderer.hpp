/**
 * Data-Driven Renderer
 *
 * Main renderer class that integrates OceanPbrApp (coregfx Vulkan rendering)
 * with data-driven shader management and gRPC real-time updates.
 *
 * Architecture:
 * - Initializes and wraps OceanPbrApp for Vulkan rendering
 * - Manages Win32 window lifecycle
 * - Bridges ShaderManager with Vulkan pipelines
 * - Handles render loop and frame rendering
 */

#pragma once

#include "Config.hpp"
#include "ShaderManager.hpp"
#include "GeometryFactory.hpp"
#include <coregfx/rsc/AssetLoader.hpp>
#include "MaterialOverride.hpp"

// Windows includes - define guards BEFORE including windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// Fix Windows macro conflicts with protobuf enums
#ifdef OPAQUE
#undef OPAQUE
#endif

#include <arctic_common.pb.h>
#include <arctic_authoring.pb.h>
#include <arctic_runtime.pb.h>
#include <arctic_network.pb.h>
#include <coregfx/pbr/pbr_app.hpp>
#include <coregfx/core/ocean_core.hpp>
#include <coregfx/gltf/gltf_service.hpp>
#include <tinygltf/tiny_gltf.h>
#include <string>
#include <memory>
#include <vector>
#include <tuple>

namespace datadriven {

/**
 * Application settings for renderer initialization
 */
struct AppSettings {
    std::string configFile = "arctic.xml";
    std::string coregfxConfigFile = "arctic.config.xml";
    bool enableGrpc = true;
    bool studioMode = false;
    glm::vec3 cameraPosition = glm::vec3(0.0f, 3.0f, 15.0f);
    glm::vec3 cameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float cameraFov = 60.0f;
};

/**
 * Main data-driven renderer class
 *
 * Integrates coregfx (OceanPbrApp) with data-driven shader management.
 * Inspired by elyrion.coregfx.renderer's Application and CoregfxManager.
 */
class Renderer {
public:
    Renderer(const AppSettings& settings, int argc, char** argv);
    ~Renderer();

    /**
     * Initialize Vulkan, window, and rendering system
     * @param hInstance Win32 instance handle
     * @return true if initialization successful
     */
    bool initialize(HINSTANCE hInstance);

    /**
     * Render a single frame
     * Updates shader instances and executes Vulkan rendering
     */
    void renderFrame();

    /**
     * Check if renderer is properly initialized and ready
     */
    bool isInitialized() const;

    /**
     * Check if application should close (window closed, etc.)
     */
    bool shouldClose() const;

    /**
     * Shutdown and cleanup resources
     */
    void shutdown();

    /**
     * Update shader instance from gRPC message
     * Applies uniform changes to Vulkan descriptors
     */
    void updateShaderInstance(const arctic::network::ShaderUpdate& update);

    /**
     * Update camera position and rotation
     */
    void updateCamera(const glm::vec3& position, const glm::vec3& rotation);

    /**
     * Get access to underlying PBR app (for advanced operations)
     */
    coregfx::OceanPbrApp* getPbrApp() { return pbrApp_; }

    /**
     * Get access to shader manager
     */
    ShaderManager& getShaderManager() { return shaderManager_; }

    /**
     * Load scene from arctic.xml
     */
    bool loadScene(const SceneConfig& config);

    /**
     * Set scene config (must be called before initialize())
     */
    void setSceneConfig(const SceneConfig& config) { sceneConfig_ = config; }

    /**
     * Get frame count (for telemetry)
     */
    uint64_t getFrameCount() const { return frameCount_; }

    /**
     * Get last frame time in milliseconds
     */
    float getLastFrameTime() const { return lastFrameTime_; }

private:
    // Internal initialization methods
    bool initializeWindow(HINSTANCE hInstance);
    bool initializeVulkan();
    bool initializeCoregfx(HINSTANCE hInstance);
    bool loadShaderTemplatesAndInstances(const SceneConfig& config);

    // Rendering methods
    void renderShaderInstances(VkCommandBuffer commandBuffer);

    // Window procedure for Win32 messages
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Member variables
    AppSettings settings_;
    coregfx::OceanPbrApp* pbrApp_ = nullptr;
    ShaderManager shaderManager_;

    // Command line arguments (needed for OceanPbrApp constructor)
    // Store copies to avoid dangling pointers from main's local variables
    int argc_;
    std::vector<std::string> argStrings_;  // Actual string storage
    std::vector<char*> argPointers_;       // Pointers to argStrings_ c_str()
    char** argv_;

    // Window management (window created and managed by OceanPbrApp)
    HINSTANCE hInstance_ = nullptr;
    bool initialized_ = false;
    bool shouldClose_ = false;

    // Scene data
    SceneConfig sceneConfig_;

    // Asset loading
    coregfx::AssetLoader assetLoader_;
    coregfx::AssetResolver assetResolver_;

    // Material override system (for gRPC material updates)
    MaterialOverrideManager materialOverrides_;

    // GLTF model context (matches elyrion's pattern)
    // tuple<uint32_t id, GltfStatus, tinygltf::Model>
    std::vector<std::tuple<uint32_t, GltfEngine::GltfService::GltfStatus, tinygltf::Model>> gltfModelContext_;

    // Test geometry (will be replaced with GLTF loading later)
    GeometryData testCube_;

    // Frame tracking
    uint64_t frameCount_ = 0;
    float lastFrameTime_ = 0.0f;
    std::chrono::steady_clock::time_point lastFrameTimePoint_;
};

} // namespace datadriven
