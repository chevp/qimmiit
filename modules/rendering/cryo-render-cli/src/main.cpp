// ==============================================================================
// cryo-render-cli - Main Entry Point
// Copyright (c) 2025 Patrice Chevillat
// ==============================================================================

#include "cryo_render/AssetGenerationPipeline.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace cryo::render;

// ==============================================================================
// Command Line Parser
// ==============================================================================

struct CommandLineArgs {
    std::string command;
    std::map<std::string, std::string> options;
    std::vector<std::string> positional;
};

CommandLineArgs parseCommandLine(int argc, char** argv) {
    CommandLineArgs args;

    if (argc < 2) {
        return args;
    }

    args.command = argv[1];

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.starts_with("--")) {
            std::string key = arg.substr(2);
            if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("--")) {
                args.options[key] = argv[++i];
            } else {
                args.options[key] = "true";
            }
        } else {
            args.positional.push_back(arg);
        }
    }

    return args;
}

// ==============================================================================
// Command Implementations
// ==============================================================================

void printHelp() {
    std::cout << R"(
cryo-render-cli - Headless Vulkan Asset Renderer
Copyright (c) 2025 Patrice Chevillat

USAGE:
    cryo-render-cli <command> [options]

COMMANDS:
    render              Render a single asset from job definition
    batch               Execute batch render jobs
    sprite-sheet        Generate sprite sheet from animation frames
    tileset             Generate tileset texture atlas
    list-gpus           List available GPUs
    validate            Validate job definition
    help                Show this help message

RENDER COMMAND:
    cryo-render-cli render --job <path> [--output <path>] [--gpu <index>]

    Options:
        --job <path>        Path to job JSON file (required)
        --output <path>     Override output path from job file
        --gpu <index>       GPU index to use (default: 0)
        --width <n>         Override width
        --height <n>        Override height

    Example:
        cryo-render-cli render --job jobs/player_sprite.json --output .cache/render/player.png

BATCH COMMAND:
    cryo-render-cli batch --config <path> [--parallel <n>]

    Options:
        --config <path>     Path to batch config JSON file (required)
        --parallel <n>      Max concurrent jobs (default: 4)
        --fail-fast         Stop on first error

    Example:
        cryo-render-cli batch --config jobs/rpg_assets_batch.json --parallel 8

SPRITE-SHEET COMMAND:
    cryo-render-cli sprite-sheet --config <path> [--output <path>]

    Options:
        --config <path>     Path to sprite sheet config JSON (required)
        --output <path>     Override output path

    Example:
        cryo-render-cli sprite-sheet --config jobs/character_walk.json --output assets/sprites/walk.png

TILESET COMMAND:
    cryo-render-cli tileset --config <path> [--output <path>]

    Options:
        --config <path>     Path to tileset config JSON (required)
        --output <path>     Override output path

    Example:
        cryo-render-cli tileset --config jobs/dungeon_tiles.json --output assets/tiles/dungeon.png

LIST-GPUS COMMAND:
    cryo-render-cli list-gpus

    Lists all available Vulkan-compatible GPUs with details:
    - GPU index
    - Device name
    - Device type
    - VRAM size

VALIDATE COMMAND:
    cryo-render-cli validate --job <path>

    Validates a job definition file without rendering.

    Example:
        cryo-render-cli validate --job jobs/player_sprite.json

JOB FILE FORMAT:
    {
        "job_id": "player_idle",
        "output_path": ".cache/render/player_idle.png",
        "width": 512,
        "height": 512,
        "scene_template": "scenes/character_template.arctic",
        "scene_parameters": {
            "character_color": "#FF0000",
            "animation": "idle"
        },
        "camera": {
            "fov": 45.0,
            "position": [0, 1, -3],
            "target": [0, 0.5, 0]
        },
        "frame_count": 1,
        "enable_transparency": true
    }

BATCH CONFIG FORMAT:
    {
        "output_directory": ".cache/render/",
        "max_concurrent_jobs": 4,
        "fail_fast": false,
        "jobs": [
            "jobs/player_idle.json",
            "jobs/player_walk_01.json",
            "jobs/player_walk_02.json"
        ]
    }

SPRITE SHEET CONFIG FORMAT:
    {
        "output_path": "assets/sprites/character_walk.png",
        "columns": 8,
        "rows": 1,
        "sprite_width": 128,
        "sprite_height": 128,
        "frame_template": {
            "scene_template": "scenes/character.arctic",
            "width": 128,
            "height": 128
        },
        "animations": [
            {
                "name": "walk",
                "start_frame": 0,
                "frame_count": 8,
                "duration_ms": 100,
                "loop": true
            }
        ],
        "metadata_output_path": "assets/sprites/character_walk.meta.json"
    }

EXAMPLES:
    # Render single asset
    cryo-render-cli render --job jobs/player.json

    # Batch render all assets
    cryo-render-cli batch --config jobs/all_assets.batch.json --parallel 8

    # Generate sprite sheet
    cryo-render-cli sprite-sheet --config jobs/walk_cycle.json

    # List available GPUs
    cryo-render-cli list-gpus

    # Validate job before rendering
    cryo-render-cli validate --job jobs/complex_scene.json

For more information, see: docs/cryo-render-cli.md
)" << std::endl;
}

int cmdRender(const CommandLineArgs& args) {
    if (args.options.find("job") == args.options.end()) {
        std::cerr << "ERROR: --job option is required" << std::endl;
        return 1;
    }

    std::string job_path = args.options.at("job");
    uint32_t gpu_index = 0;

    if (args.options.find("gpu") != args.options.end()) {
        gpu_index = std::stoi(args.options.at("gpu"));
    }

    try {
        std::cout << "[cryo-render] Loading job: " << job_path << std::endl;

        auto job = RenderJobDefinition::fromJSON(job_path);

        // Apply overrides
        if (args.options.find("output") != args.options.end()) {
            job.output_path = args.options.at("output");
        }

        if (args.options.find("width") != args.options.end()) {
            job.width = std::stoi(args.options.at("width"));
        }

        if (args.options.find("height") != args.options.end()) {
            job.height = std::stoi(args.options.at("height"));
        }

        // Initialize pipeline
        AssetGenerationPipeline pipeline;
        if (!pipeline.initializeHeadless(gpu_index)) {
            std::cerr << "ERROR: Failed to initialize rendering pipeline" << std::endl;
            return 1;
        }

        // Execute job
        auto result = pipeline.executeJob(job);

        if (result.success) {
            std::cout << "\n[SUCCESS] Rendered: " << result.output_path << std::endl;
            std::cout << "  Size: " << result.file_size_bytes << " bytes" << std::endl;
            std::cout << "  Render time: " << result.timing.render_time_ms << "ms" << std::endl;
            std::cout << "  Total time: " << result.timing.total_time_ms << "ms" << std::endl;
            return 0;
        } else {
            std::cerr << "\n[FAILED] " << result.error_message << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}

int cmdBatch(const CommandLineArgs& args) {
    if (args.options.find("config") == args.options.end()) {
        std::cerr << "ERROR: --config option is required" << std::endl;
        return 1;
    }

    std::string config_path = args.options.at("config");

    try {
        std::cout << "[cryo-render] Loading batch config: " << config_path << std::endl;

        auto config = BatchRenderConfig::fromJSON(config_path);

        // Apply overrides
        if (args.options.find("parallel") != args.options.end()) {
            config.max_concurrent_jobs = std::stoi(args.options.at("parallel"));
        }

        if (args.options.find("fail-fast") != args.options.end()) {
            config.fail_fast = true;
        }

        // Progress callback
        config.progress_callback = [](size_t completed, size_t total, const std::string& job_id) {
            std::cout << "\r[Progress] " << completed << "/" << total << " - " << job_id << std::flush;
        };

        // Initialize pipeline
        AssetGenerationPipeline pipeline;
        if (!pipeline.initializeHeadless()) {
            std::cerr << "ERROR: Failed to initialize rendering pipeline" << std::endl;
            return 1;
        }

        // Execute batch
        auto results = pipeline.executeBatch(config);

        // Summary
        size_t succeeded = 0;
        size_t failed = 0;
        for (const auto& result : results) {
            if (result.success) succeeded++;
            else failed++;
        }

        std::cout << "\n\n[BATCH COMPLETE]" << std::endl;
        std::cout << "  Total: " << results.size() << std::endl;
        std::cout << "  Succeeded: " << succeeded << std::endl;
        std::cout << "  Failed: " << failed << std::endl;

        auto stats = pipeline.getStatistics();
        std::cout << "  Total render time: " << stats.total_render_time_ms << "ms" << std::endl;

        return (failed == 0) ? 0 : 1;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}

int cmdSpriteSheet(const CommandLineArgs& args) {
    if (args.options.find("config") == args.options.end()) {
        std::cerr << "ERROR: --config option is required" << std::endl;
        return 1;
    }

    std::string config_path = args.options.at("config");

    try {
        std::cout << "[cryo-render] Loading sprite sheet config: " << config_path << std::endl;

        auto definition = SpriteSheetDefinition::fromJSON(config_path);

        // Apply overrides
        if (args.options.find("output") != args.options.end()) {
            definition.output_path = args.options.at("output");
        }

        // Initialize pipeline
        AssetGenerationPipeline pipeline;
        if (!pipeline.initializeHeadless()) {
            std::cerr << "ERROR: Failed to initialize rendering pipeline" << std::endl;
            return 1;
        }

        // Generate sprite sheet
        auto result = pipeline.generateSpriteSheet(definition);

        if (result.success) {
            std::cout << "\n[SUCCESS] Generated sprite sheet: " << result.output_path << std::endl;
            return 0;
        } else {
            std::cerr << "\n[FAILED] " << result.error_message << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}

int cmdTileSet(const CommandLineArgs& args) {
    if (args.options.find("config") == args.options.end()) {
        std::cerr << "ERROR: --config option is required" << std::endl;
        return 1;
    }

    std::string config_path = args.options.at("config");

    try {
        std::cout << "[cryo-render] Loading tileset config: " << config_path << std::endl;

        auto definition = TileSetDefinition::fromJSON(config_path);

        // Apply overrides
        if (args.options.find("output") != args.options.end()) {
            definition.output_path = args.options.at("output");
        }

        // Initialize pipeline
        AssetGenerationPipeline pipeline;
        if (!pipeline.initializeHeadless()) {
            std::cerr << "ERROR: Failed to initialize rendering pipeline" << std::endl;
            return 1;
        }

        // Generate tileset
        auto result = pipeline.generateTileSet(definition);

        if (result.success) {
            std::cout << "\n[SUCCESS] Generated tileset: " << result.output_path << std::endl;
            return 0;
        } else {
            std::cerr << "\n[FAILED] " << result.error_message << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}

int cmdListGPUs(const CommandLineArgs& args) {
    auto gpus = AssetGenerationPipeline::enumerateGPUs();

    if (gpus.empty()) {
        std::cout << "No Vulkan-compatible GPUs found" << std::endl;
        return 1;
    }

    std::cout << "\nAvailable GPUs:\n" << std::endl;

    for (const auto& gpu : gpus) {
        std::cout << "  [" << gpu.index << "] " << gpu.name << std::endl;
        std::cout << "      Type: ";
        switch (gpu.type) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                std::cout << "Discrete GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                std::cout << "Integrated GPU";
                break;
            default:
                std::cout << "Other";
                break;
        }
        std::cout << std::endl;
        std::cout << "      VRAM: " << gpu.vram_size_mb << " MB" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

int cmdValidate(const CommandLineArgs& args) {
    if (args.options.find("job") == args.options.end()) {
        std::cerr << "ERROR: --job option is required" << std::endl;
        return 1;
    }

    std::string job_path = args.options.at("job");

    try {
        std::cout << "[cryo-render] Validating job: " << job_path << std::endl;

        auto job = RenderJobDefinition::fromJSON(job_path);

        std::string error_message;
        if (AssetGenerationPipeline::validateJob(job, error_message)) {
            std::cout << "[SUCCESS] Job definition is valid" << std::endl;
            std::cout << "  Job ID: " << job.job_id << std::endl;
            std::cout << "  Output: " << job.output_path << std::endl;
            std::cout << "  Resolution: " << job.width << "x" << job.height << std::endl;
            std::cout << "  Scene: " << job.scene_template << std::endl;
            return 0;
        } else {
            std::cerr << "[FAILED] Validation error: " << error_message << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}

// ==============================================================================
// Main Entry Point
// ==============================================================================

int main(int argc, char** argv) {
    std::cout << "cryo-render-cli v" << CRYO_RENDER_VERSION << std::endl;
    std::cout << "Copyright (c) 2025 Patrice Chevillat\n" << std::endl;

    auto args = parseCommandLine(argc, argv);

    if (args.command.empty() || args.command == "help") {
        printHelp();
        return 0;
    }

    if (args.command == "render") {
        return cmdRender(args);
    } else if (args.command == "batch") {
        return cmdBatch(args);
    } else if (args.command == "sprite-sheet") {
        return cmdSpriteSheet(args);
    } else if (args.command == "tileset") {
        return cmdTileSet(args);
    } else if (args.command == "list-gpus") {
        return cmdListGPUs(args);
    } else if (args.command == "validate") {
        return cmdValidate(args);
    } else {
        std::cerr << "ERROR: Unknown command: " << args.command << std::endl;
        std::cerr << "Run 'cryo-render-cli help' for usage information" << std::endl;
        return 1;
    }
}