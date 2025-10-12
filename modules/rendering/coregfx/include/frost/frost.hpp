// frost.hpp - Direct C++ API for Frost Renderer (No gRPC)
// Purpose: Embed Frost renderer directly into your application as a library
// Usage: #include <frost/frost.hpp> and link against frost.lib/frost.dll
#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>

// Forward declare proto types to avoid exposing frostapi.pb.h in this header
namespace frost {
    class FrostResponse;
    class InitializeRequest;
    class LoadSceneRequest;
    class AddEntityRequest;
    class UpdateEntityRequest;
    class SetCameraRequest;
    class GetCameraResponse;
    class CaptureScreenshotRequest;
    class ScreenshotResponse;
    class LoadGLTFRequest;
    class SetMaterialRequest;
    class SetLightingRequest;
    class GetStateResponse;
    enum FrostError : int;
}

namespace coregfx {

// Simple C++ types (no proto exposure)
struct Vec3 {
    float x, y, z;
    Vec3(float x_ = 0, float y_ = 0, float z_ = 0) : x(x_), y(y_), z(z_) {}
};

struct Vec4 {
    float x, y, z, w;
    Vec4(float x_ = 0, float y_ = 0, float z_ = 0, float w_ = 1)
        : x(x_), y(y_), z(z_), w(w_) {}
};

struct Transform {
    Vec3 position;
    Vec3 rotation;  // Euler angles in degrees
    Vec3 scale;
    Transform() : scale(1, 1, 1) {}
};

struct FrostInitParams {
    int width = 1920;
    int height = 1080;
    bool headless = false;
    std::string window_title = "Frost Renderer";
    bool enable_validation = false;
    bool enable_vsync = true;
    int msaa_samples = 1;
    std::string asset_root = "assets/vkpbr5";
    std::string shader_dir = "assets/vkpbr5/shaders";
    std::string config_file_path = "arctic.config.xml";
};

struct FrostResult {
    bool success;
    std::string message;
    int error_code;  // Maps to frost::FrostError enum
};

struct CameraState {
    Vec3 position;
    Vec3 rotation;
    float fov;
    float near_plane;
    float far_plane;
};

// ============================================================================
// Main Frost Renderer Class - Direct Library API
// ============================================================================

class FrostRenderer {
public:
    FrostRenderer();
    ~FrostRenderer();

    // Lifecycle
    FrostResult Initialize(const FrostInitParams& params);
    FrostResult Shutdown();
    bool IsInitialized() const;

    // Scene Management
    FrostResult LoadScene(const std::string& file_path, bool clear_existing = true);
    FrostResult LoadSceneFromMemory(const std::vector<uint8_t>& scene_data);
    FrostResult UnloadScene(const std::string& scene_id = "");
    FrostResult ClearScene(bool keep_camera = false, bool keep_lighting = false);

    // Entity Management
    FrostResult AddEntity(const std::string& entity_id,
                         const std::string& gltf_id,
                         const Transform& transform = Transform(),
                         const std::map<std::string, std::string>& properties = {});

    FrostResult RemoveEntity(const std::string& entity_id);

    FrostResult UpdateEntity(const std::string& entity_id,
                            const Transform& transform,
                            bool update_position = true,
                            bool update_rotation = true,
                            bool update_scale = true);

    FrostResult BatchUpdateEntities(const std::map<std::string, Transform>& updates);

    // Camera Control
    FrostResult SetCamera(const Vec3& position,
                         const Vec3& rotation,
                         float fov = 60.0f,
                         float near_plane = 0.1f,
                         float far_plane = 1000.0f);

    CameraState GetCamera() const;

    // Rendering
    FrostResult RenderFrame(bool blocking = false);
    FrostResult CaptureScreenshot(const std::string& filename,
                                 int width = 0,  // 0 = use current resolution
                                 int height = 0,
                                 const std::string& format = "png");

    // Asset Loading
    FrostResult LoadGLTF(const std::string& gltf_id, const std::string& file_path);
    FrostResult LoadGLTFFromMemory(const std::string& gltf_id,
                                   const std::vector<uint8_t>& gltf_data);
    FrostResult UnloadGLTF(const std::string& gltf_id);

    // Material & Lighting
    FrostResult SetMaterial(const std::string& entity_id,
                           const Vec4& base_color,
                           float metallic = 0.0f,
                           float roughness = 0.5f,
                           float emissive_strength = 0.0f);

    FrostResult SetLighting(const Vec3& light_direction,
                           const Vec3& light_color,
                           float light_intensity = 1.0f,
                           const std::string& environment_map = "");

    // State Query
    int GetEntityCount() const;
    std::vector<std::string> GetEntityIds() const;
    std::vector<std::string> GetLoadedGLTFIds() const;
    int GetFrameCount() const;
    float GetFPS() const;

    // Advanced
    std::string DumpStateToString(const std::string& format = "compact") const;
    FrostResult DumpStateToFile(const std::string& output_file,
                               const std::string& format = "compact");

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;  // Hide all coregfx/Vulkan details
};

} // namespace coregfx
