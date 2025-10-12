#pragma once

#include <cstdint>

namespace coregfx {

/**
 * Shared rendering constants for coregfx library
 *
 * ONLY graphics/rendering-related constants belong here!
 *
 * Application-specific constants should stay in application code:
 * - HTTP server config → application code (cryo-studio-server, etc.)
 * - File paths → cgfx.proto (ConfigPaths message)
 * - Error codes → cgfx.proto (CryoErrorCode enum)
 * - Logging config → cgfx.proto (DebugConfig message)
 */
namespace RendererConstants {

    // Camera Configuration
    namespace Camera {
        constexpr float DEFAULT_POSITION_X = 0.0f;
        constexpr float DEFAULT_POSITION_Y = 0.0f;
        constexpr float DEFAULT_POSITION_Z = 10.0f;  // Safe distance from origin
        constexpr float DEFAULT_ROTATION_X = 0.0f;
        constexpr float DEFAULT_ROTATION_Y = 0.0f;
        constexpr float DEFAULT_ROTATION_Z = 0.0f;
        constexpr float DEFAULT_FOV = 45.0f;
        constexpr float DEFAULT_NEAR_PLANE = 0.1f;
        constexpr float DEFAULT_FAR_PLANE = 1000.0f;
        constexpr float DEFAULT_ASPECT_RATIO = 1.777f;  // 16:9
    }

    // Vulkan Graphics
    namespace Graphics {
        constexpr int DEFAULT_WINDOW_WIDTH = 1920;
        constexpr int DEFAULT_WINDOW_HEIGHT = 1080;
        constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        constexpr size_t UNIFORM_BUFFER_ALIGNMENT = 256;
        constexpr int DEFAULT_ANISOTROPY = 16;
        constexpr int DEFAULT_MSAA_SAMPLES = 4;
    }

    // Timing (for animations, frame timing, etc.)
    namespace Timing {
        constexpr int64_t MILLISECONDS_PER_SECOND = 1000;
        constexpr int64_t NANOSECONDS_PER_SECOND = 1000000000;
        constexpr double WINDOWS_FILETIME_TO_SECONDS = 10000000.0;  // 100ns intervals
        constexpr int DEFAULT_DUMP_DELAY_SECONDS = 5;
    }

} // namespace RendererConstants

} // namespace coregfx
