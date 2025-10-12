// ==============================================================================
// AssetGenerationPipeline.hpp - High-performance asset rendering pipeline
// Copyright (c) 2025 Patrice Chevillat
// ==============================================================================

#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

namespace cryo::render {

// ==============================================================================
// Forward Declarations
// ==============================================================================

class HeadlessRenderer;
class FramebufferCapture;

// ==============================================================================
// Render Job Definition
// ==============================================================================

struct RenderJobDefinition {
    std::string job_id;
    std::string output_path;

    // Render target configuration
    uint32_t width = 1024;
    uint32_t height = 1024;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    uint32_t sample_count = 1;  // MSAA samples

    // Scene configuration
    std::string scene_template;      // Path to .arctic scene or JSON template
    nlohmann::json scene_parameters; // Dynamic parameters (colors, positions, etc.)

    // Camera configuration
    struct {
        float fov = 45.0f;
        float near_plane = 0.1f;
        float far_plane = 100.0f;
        std::array<float, 3> position = {0.0f, 0.0f, -5.0f};
        std::array<float, 3> target = {0.0f, 0.0f, 0.0f};
    } camera;

    // Animation/variants
    std::vector<std::string> variants;  // e.g., ["red", "blue", "green"]
    uint32_t frame_count = 1;           // For sprite sheets
    float frame_duration_ms = 100.0f;   // Animation frame duration

    // Post-processing
    bool enable_transparency = true;
    bool premultiply_alpha = false;

    // Metadata
    nlohmann::json custom_metadata;

    // Load from JSON file
    static RenderJobDefinition fromJSON(const std::string& json_path);

    // Save to JSON file
    void toJSON(const std::string& json_path) const;
};

// ==============================================================================
// Batch Render Configuration
// ==============================================================================

struct BatchRenderConfig {
    std::vector<RenderJobDefinition> jobs;

    // Execution settings
    uint32_t max_concurrent_jobs = 4;
    bool fail_fast = false;  // Stop on first error

    // Output settings
    std::string output_directory = ".cache/render/";
    bool create_output_dirs = true;

    // Progress callback
    using ProgressCallback = std::function<void(size_t completed, size_t total, const std::string& current_job)>;
    ProgressCallback progress_callback;

    // Load batch from JSON file
    static BatchRenderConfig fromJSON(const std::string& json_path);
};

// ==============================================================================
// Sprite Sheet Configuration
// ==============================================================================

struct SpriteSheetDefinition {
    std::string output_path;

    // Grid layout
    uint32_t columns = 8;
    uint32_t rows = 1;
    uint32_t sprite_width = 128;
    uint32_t sprite_height = 128;
    uint32_t padding = 0;  // Space between sprites

    // Animation
    struct AnimationSequence {
        std::string name;
        uint32_t start_frame = 0;
        uint32_t frame_count = 1;
        float duration_ms = 100.0f;
        bool loop = true;
    };

    std::vector<AnimationSequence> animations;

    // Scene configuration for each frame
    RenderJobDefinition frame_template;

    // Per-frame overrides (e.g., rotation, position changes)
    std::vector<nlohmann::json> frame_overrides;

    // Metadata export
    std::string metadata_output_path;  // JSON metadata for sprite coordinates

    static SpriteSheetDefinition fromJSON(const std::string& json_path);
};

// ==============================================================================
// Tile Set Configuration
// ==============================================================================

struct TileSetDefinition {
    std::string output_path;

    // Tile grid
    uint32_t tile_width = 64;
    uint32_t tile_height = 64;
    uint32_t columns = 16;
    uint32_t rows = 16;

    // Tile definitions (each tile is a separate render job)
    struct TileDefinition {
        uint32_t tile_id;
        std::string tile_name;
        RenderJobDefinition render_job;
    };

    std::vector<TileDefinition> tiles;

    // Auto-variants (e.g., rotations, flips)
    bool generate_rotations = false;
    bool generate_flips = false;

    // Metadata export
    std::string metadata_output_path;

    static TileSetDefinition fromJSON(const std::string& json_path);
};

// ==============================================================================
// Render Job Result
// ==============================================================================

struct RenderJobResult {
    std::string job_id;
    bool success = false;
    std::string error_message;

    // Output file info
    std::string output_path;
    size_t file_size_bytes = 0;

    // Render statistics
    struct {
        double render_time_ms = 0.0;
        double capture_time_ms = 0.0;
        double encode_time_ms = 0.0;
        double total_time_ms = 0.0;
    } timing;

    // Metadata
    nlohmann::json metadata;
};

// ==============================================================================
// Asset Generation Pipeline
// ==============================================================================

class AssetGenerationPipeline {
public:
    AssetGenerationPipeline();
    ~AssetGenerationPipeline();

    // Initialize headless Vulkan context
    bool initializeHeadless(uint32_t preferred_gpu_index = 0);

    // Shutdown and cleanup
    void shutdown();

    // =========================================================================
    // Single Render Operations
    // =========================================================================

    // Execute a single render job
    RenderJobResult executeJob(const RenderJobDefinition& job);

    // Render to memory buffer (no file output)
    std::vector<uint8_t> renderToBuffer(const RenderJobDefinition& job);

    // =========================================================================
    // Batch Operations
    // =========================================================================

    // Execute multiple jobs (parallel or sequential)
    std::vector<RenderJobResult> executeBatch(const BatchRenderConfig& config);

    // Execute batch from JSON file
    std::vector<RenderJobResult> executeBatchFromFile(const std::string& json_path);

    // =========================================================================
    // Sprite Sheet Generation
    // =========================================================================

    // Generate sprite sheet with multiple frames
    RenderJobResult generateSpriteSheet(const SpriteSheetDefinition& definition);

    // Generate sprite sheet from JSON file
    RenderJobResult generateSpriteSheetFromFile(const std::string& json_path);

    // =========================================================================
    // Tile Set Generation
    // =========================================================================

    // Generate tile set texture
    RenderJobResult generateTileSet(const TileSetDefinition& definition);

    // Generate tile set from JSON file
    RenderJobResult generateTileSetFromFile(const std::string& json_path);

    // =========================================================================
    // Utilities
    // =========================================================================

    // Get available GPUs
    struct GPUInfo {
        uint32_t index;
        std::string name;
        VkPhysicalDeviceType type;
        uint64_t vram_size_mb;
    };

    static std::vector<GPUInfo> enumerateGPUs();

    // Validate job definition
    static bool validateJob(const RenderJobDefinition& job, std::string& error_message);

    // Get pipeline statistics
    struct Statistics {
        uint32_t jobs_executed = 0;
        uint32_t jobs_succeeded = 0;
        uint32_t jobs_failed = 0;
        double total_render_time_ms = 0.0;
        size_t total_output_bytes = 0;
    };

    Statistics getStatistics() const { return statistics_; }
    void resetStatistics() { statistics_ = Statistics{}; }

private:
    // Internal implementation
    struct Impl;
    std::unique_ptr<Impl> impl_;

    // Statistics
    Statistics statistics_;

    // Helper methods
    bool setupRenderTarget(const RenderJobDefinition& job);
    bool renderScene(const RenderJobDefinition& job);
    bool captureFramebuffer(const std::string& output_path);
    bool exportMetadata(const std::string& output_path, const nlohmann::json& metadata);
};

// ==============================================================================
// Convenience Functions
// ==============================================================================

// Quick render to PNG
bool quickRenderToPNG(
    const std::string& scene_path,
    const std::string& output_path,
    uint32_t width = 1024,
    uint32_t height = 1024
);

// Generate sprite sheet from directory of scene files
bool generateSpriteSheetFromScenes(
    const std::vector<std::string>& scene_paths,
    const std::string& output_path,
    uint32_t sprite_width = 128,
    uint32_t sprite_height = 128
);

} // namespace cryo::render