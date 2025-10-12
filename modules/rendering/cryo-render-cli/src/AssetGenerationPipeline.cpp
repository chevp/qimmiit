// ==============================================================================
// AssetGenerationPipeline.cpp - Implementation
// Copyright (c) 2025 Patrice Chevillat
// ==============================================================================

#include "cryo_render/AssetGenerationPipeline.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iostream>

// stb_image_write for PNG encoding
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace cryo::render {

namespace fs = std::filesystem;

// ==============================================================================
// RenderJobDefinition Implementation
// ==============================================================================

RenderJobDefinition RenderJobDefinition::fromJSON(const std::string& json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open job file: " + json_path);
    }

    nlohmann::json j;
    file >> j;

    RenderJobDefinition job;
    job.job_id = j.value("job_id", fs::path(json_path).stem().string());
    job.output_path = j.value("output_path", "");
    job.width = j.value("width", 1024);
    job.height = j.value("height", 1024);
    job.scene_template = j.value("scene_template", "");

    if (j.contains("scene_parameters")) {
        job.scene_parameters = j["scene_parameters"];
    }

    if (j.contains("camera")) {
        auto& cam = j["camera"];
        job.camera.fov = cam.value("fov", 45.0f);
        job.camera.near_plane = cam.value("near", 0.1f);
        job.camera.far_plane = cam.value("far", 100.0f);

        if (cam.contains("position")) {
            auto pos = cam["position"];
            job.camera.position = {pos[0], pos[1], pos[2]};
        }

        if (cam.contains("target")) {
            auto tgt = cam["target"];
            job.camera.target = {tgt[0], tgt[1], tgt[2]};
        }
    }

    if (j.contains("variants")) {
        job.variants = j["variants"].get<std::vector<std::string>>();
    }

    job.frame_count = j.value("frame_count", 1);
    job.frame_duration_ms = j.value("frame_duration_ms", 100.0f);
    job.enable_transparency = j.value("enable_transparency", true);

    if (j.contains("metadata")) {
        job.custom_metadata = j["metadata"];
    }

    return job;
}

void RenderJobDefinition::toJSON(const std::string& json_path) const {
    nlohmann::json j;
    j["job_id"] = job_id;
    j["output_path"] = output_path;
    j["width"] = width;
    j["height"] = height;
    j["scene_template"] = scene_template;
    j["scene_parameters"] = scene_parameters;

    j["camera"] = {
        {"fov", camera.fov},
        {"near", camera.near_plane},
        {"far", camera.far_plane},
        {"position", camera.position},
        {"target", camera.target}
    };

    j["variants"] = variants;
    j["frame_count"] = frame_count;
    j["frame_duration_ms"] = frame_duration_ms;
    j["enable_transparency"] = enable_transparency;
    j["metadata"] = custom_metadata;

    std::ofstream file(json_path);
    file << j.dump(4);
}

// ==============================================================================
// BatchRenderConfig Implementation
// ==============================================================================

BatchRenderConfig BatchRenderConfig::fromJSON(const std::string& json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open batch file: " + json_path);
    }

    nlohmann::json j;
    file >> j;

    BatchRenderConfig config;
    config.output_directory = j.value("output_directory", ".cache/render/");
    config.max_concurrent_jobs = j.value("max_concurrent_jobs", 4);
    config.fail_fast = j.value("fail_fast", false);

    if (j.contains("jobs")) {
        for (const auto& job_json : j["jobs"]) {
            // If job is a string, treat it as a file path
            if (job_json.is_string()) {
                config.jobs.push_back(RenderJobDefinition::fromJSON(job_json));
            } else {
                // Inline job definition
                std::string temp_path = ".temp_job.json";
                std::ofstream temp_file(temp_path);
                temp_file << job_json.dump();
                temp_file.close();
                config.jobs.push_back(RenderJobDefinition::fromJSON(temp_path));
                fs::remove(temp_path);
            }
        }
    }

    return config;
}

// ==============================================================================
// SpriteSheetDefinition Implementation
// ==============================================================================

SpriteSheetDefinition SpriteSheetDefinition::fromJSON(const std::string& json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open sprite sheet file: " + json_path);
    }

    nlohmann::json j;
    file >> j;

    SpriteSheetDefinition def;
    def.output_path = j.value("output_path", "");
    def.columns = j.value("columns", 8);
    def.rows = j.value("rows", 1);
    def.sprite_width = j.value("sprite_width", 128);
    def.sprite_height = j.value("sprite_height", 128);
    def.padding = j.value("padding", 0);
    def.metadata_output_path = j.value("metadata_output_path", "");

    if (j.contains("frame_template")) {
        std::string temp_path = ".temp_frame_template.json";
        std::ofstream temp_file(temp_path);
        temp_file << j["frame_template"].dump();
        temp_file.close();
        def.frame_template = RenderJobDefinition::fromJSON(temp_path);
        fs::remove(temp_path);
    }

    if (j.contains("animations")) {
        for (const auto& anim_json : j["animations"]) {
            AnimationSequence anim;
            anim.name = anim_json.value("name", "");
            anim.start_frame = anim_json.value("start_frame", 0);
            anim.frame_count = anim_json.value("frame_count", 1);
            anim.duration_ms = anim_json.value("duration_ms", 100.0f);
            anim.loop = anim_json.value("loop", true);
            def.animations.push_back(anim);
        }
    }

    if (j.contains("frame_overrides")) {
        def.frame_overrides = j["frame_overrides"].get<std::vector<nlohmann::json>>();
    }

    return def;
}

// ==============================================================================
// TileSetDefinition Implementation
// ==============================================================================

TileSetDefinition TileSetDefinition::fromJSON(const std::string& json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open tileset file: " + json_path);
    }

    nlohmann::json j;
    file >> j;

    TileSetDefinition def;
    def.output_path = j.value("output_path", "");
    def.tile_width = j.value("tile_width", 64);
    def.tile_height = j.value("tile_height", 64);
    def.columns = j.value("columns", 16);
    def.rows = j.value("rows", 16);
    def.generate_rotations = j.value("generate_rotations", false);
    def.generate_flips = j.value("generate_flips", false);
    def.metadata_output_path = j.value("metadata_output_path", "");

    if (j.contains("tiles")) {
        for (const auto& tile_json : j["tiles"]) {
            TileDefinition tile;
            tile.tile_id = tile_json.value("tile_id", 0);
            tile.tile_name = tile_json.value("tile_name", "");

            if (tile_json.contains("render_job")) {
                std::string temp_path = ".temp_tile_job.json";
                std::ofstream temp_file(temp_path);
                temp_file << tile_json["render_job"].dump();
                temp_file.close();
                tile.render_job = RenderJobDefinition::fromJSON(temp_path);
                fs::remove(temp_path);
            }

            def.tiles.push_back(tile);
        }
    }

    return def;
}

// ==============================================================================
// AssetGenerationPipeline Implementation
// ==============================================================================

struct AssetGenerationPipeline::Impl {
    // Vulkan context (headless)
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    uint32_t graphics_queue_family = 0;

    // Command pool and buffers
    VkCommandPool command_pool = VK_NULL_HANDLE;

    // Offscreen rendering resources
    VkImage offscreen_image = VK_NULL_HANDLE;
    VkDeviceMemory offscreen_memory = VK_NULL_HANDLE;
    VkImageView offscreen_image_view = VK_NULL_HANDLE;
    VkFramebuffer offscreen_framebuffer = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;

    // Staging buffer for readback
    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VkDeviceMemory staging_memory = VK_NULL_HANDLE;
    size_t staging_buffer_size = 0;

    bool initialized = false;
};

AssetGenerationPipeline::AssetGenerationPipeline()
    : impl_(std::make_unique<Impl>()) {
}

AssetGenerationPipeline::~AssetGenerationPipeline() {
    shutdown();
}

bool AssetGenerationPipeline::initializeHeadless(uint32_t preferred_gpu_index) {
    std::cout << "[cryo-render] Initializing headless Vulkan context..." << std::endl;

    // TODO: Full Vulkan initialization
    // For now, this is a placeholder showing the structure

    // 1. Create Vulkan instance (no surface)
    // 2. Select physical device
    // 3. Create logical device
    // 4. Get graphics queue
    // 5. Create command pool

    impl_->initialized = true;
    std::cout << "[cryo-render] Headless context initialized successfully" << std::endl;
    return true;
}

void AssetGenerationPipeline::shutdown() {
    if (!impl_->initialized) return;

    std::cout << "[cryo-render] Shutting down pipeline..." << std::endl;

    // TODO: Cleanup Vulkan resources
    // vkDestroyFramebuffer, vkDestroyImageView, etc.

    impl_->initialized = false;
}

RenderJobResult AssetGenerationPipeline::executeJob(const RenderJobDefinition& job) {
    RenderJobResult result;
    result.job_id = job.job_id;

    auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "[cryo-render] Executing job: " << job.job_id << std::endl;
    std::cout << "  Output: " << job.output_path << std::endl;
    std::cout << "  Resolution: " << job.width << "x" << job.height << std::endl;

    try {
        // 1. Setup render target
        auto render_start = std::chrono::high_resolution_clock::now();
        if (!setupRenderTarget(job)) {
            throw std::runtime_error("Failed to setup render target");
        }

        // 2. Render scene
        if (!renderScene(job)) {
            throw std::runtime_error("Failed to render scene");
        }
        auto render_end = std::chrono::high_resolution_clock::now();
        result.timing.render_time_ms = std::chrono::duration<double, std::milli>(render_end - render_start).count();

        // 3. Capture framebuffer
        auto capture_start = std::chrono::high_resolution_clock::now();
        if (!captureFramebuffer(job.output_path)) {
            throw std::runtime_error("Failed to capture framebuffer");
        }
        auto capture_end = std::chrono::high_resolution_clock::now();
        result.timing.capture_time_ms = std::chrono::duration<double, std::milli>(capture_end - capture_start).count();

        // 4. Export metadata
        if (!job.custom_metadata.empty()) {
            std::string meta_path = job.output_path + ".meta.json";
            exportMetadata(meta_path, job.custom_metadata);
        }

        result.success = true;
        result.output_path = job.output_path;

        if (fs::exists(job.output_path)) {
            result.file_size_bytes = fs::file_size(job.output_path);
        }

        statistics_.jobs_executed++;
        statistics_.jobs_succeeded++;

    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
        statistics_.jobs_executed++;
        statistics_.jobs_failed++;
        std::cerr << "[cryo-render] ERROR: " << e.what() << std::endl;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.timing.total_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    std::cout << "[cryo-render] Job completed in " << result.timing.total_time_ms << "ms" << std::endl;

    return result;
}

std::vector<uint8_t> AssetGenerationPipeline::renderToBuffer(const RenderJobDefinition& job) {
    // TODO: Render directly to memory buffer without file I/O
    std::vector<uint8_t> buffer;
    return buffer;
}

std::vector<RenderJobResult> AssetGenerationPipeline::executeBatch(const BatchRenderConfig& config) {
    std::vector<RenderJobResult> results;

    std::cout << "[cryo-render] Starting batch render: " << config.jobs.size() << " jobs" << std::endl;

    size_t completed = 0;
    for (const auto& job : config.jobs) {
        auto result = executeJob(job);
        results.push_back(result);

        completed++;
        if (config.progress_callback) {
            config.progress_callback(completed, config.jobs.size(), job.job_id);
        }

        if (config.fail_fast && !result.success) {
            std::cerr << "[cryo-render] Batch failed (fail_fast enabled)" << std::endl;
            break;
        }
    }

    std::cout << "[cryo-render] Batch complete: " << completed << "/" << config.jobs.size() << " jobs" << std::endl;
    return results;
}

std::vector<RenderJobResult> AssetGenerationPipeline::executeBatchFromFile(const std::string& json_path) {
    auto config = BatchRenderConfig::fromJSON(json_path);
    return executeBatch(config);
}

RenderJobResult AssetGenerationPipeline::generateSpriteSheet(const SpriteSheetDefinition& definition) {
    RenderJobResult result;
    result.job_id = "sprite_sheet_" + fs::path(definition.output_path).stem().string();

    std::cout << "[cryo-render] Generating sprite sheet: " << definition.columns << "x" << definition.rows << std::endl;

    // TODO: Implement sprite sheet generation
    // 1. Calculate total frame count
    // 2. Render each frame individually
    // 3. Composite into single atlas image
    // 4. Export metadata

    result.success = false;
    result.error_message = "Not yet implemented";
    return result;
}

RenderJobResult AssetGenerationPipeline::generateSpriteSheetFromFile(const std::string& json_path) {
    auto definition = SpriteSheetDefinition::fromJSON(json_path);
    return generateSpriteSheet(definition);
}

RenderJobResult AssetGenerationPipeline::generateTileSet(const TileSetDefinition& definition) {
    RenderJobResult result;
    result.job_id = "tileset_" + fs::path(definition.output_path).stem().string();

    std::cout << "[cryo-render] Generating tileset: " << definition.columns << "x" << definition.rows << std::endl;

    // TODO: Implement tileset generation

    result.success = false;
    result.error_message = "Not yet implemented";
    return result;
}

RenderJobResult AssetGenerationPipeline::generateTileSetFromFile(const std::string& json_path) {
    auto definition = TileSetDefinition::fromJSON(json_path);
    return generateTileSet(definition);
}

std::vector<AssetGenerationPipeline::GPUInfo> AssetGenerationPipeline::enumerateGPUs() {
    std::vector<GPUInfo> gpus;

    // TODO: Enumerate Vulkan physical devices

    return gpus;
}

bool AssetGenerationPipeline::validateJob(const RenderJobDefinition& job, std::string& error_message) {
    if (job.output_path.empty()) {
        error_message = "Output path is empty";
        return false;
    }

    if (job.width == 0 || job.height == 0) {
        error_message = "Invalid resolution";
        return false;
    }

    if (job.scene_template.empty()) {
        error_message = "Scene template is required";
        return false;
    }

    return true;
}

// Private helper methods

bool AssetGenerationPipeline::setupRenderTarget(const RenderJobDefinition& job) {
    // TODO: Create offscreen framebuffer with job.width x job.height
    return true;
}

bool AssetGenerationPipeline::renderScene(const RenderJobDefinition& job) {
    // TODO: Load scene and render to offscreen framebuffer
    return true;
}

bool AssetGenerationPipeline::captureFramebuffer(const std::string& output_path) {
    // TODO: Download framebuffer data and encode to PNG using stb_image_write

    // Placeholder: create a dummy PNG
    std::vector<uint8_t> dummy_pixels(1024 * 1024 * 4, 128); // Gray image

    // Ensure output directory exists
    fs::create_directories(fs::path(output_path).parent_path());

    int result = stbi_write_png(
        output_path.c_str(),
        1024, 1024,
        4, // RGBA
        dummy_pixels.data(),
        1024 * 4 // stride
    );

    return result != 0;
}

bool AssetGenerationPipeline::exportMetadata(const std::string& output_path, const nlohmann::json& metadata) {
    std::ofstream file(output_path);
    if (!file.is_open()) return false;

    file << metadata.dump(4);
    return true;
}

// ==============================================================================
// Convenience Functions
// ==============================================================================

bool quickRenderToPNG(
    const std::string& scene_path,
    const std::string& output_path,
    uint32_t width,
    uint32_t height
) {
    AssetGenerationPipeline pipeline;
    if (!pipeline.initializeHeadless()) {
        return false;
    }

    RenderJobDefinition job;
    job.job_id = "quick_render";
    job.scene_template = scene_path;
    job.output_path = output_path;
    job.width = width;
    job.height = height;

    auto result = pipeline.executeJob(job);
    return result.success;
}

bool generateSpriteSheetFromScenes(
    const std::vector<std::string>& scene_paths,
    const std::string& output_path,
    uint32_t sprite_width,
    uint32_t sprite_height
) {
    // TODO: Implement
    return false;
}

} // namespace cryo::render