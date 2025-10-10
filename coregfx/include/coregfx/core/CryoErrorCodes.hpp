#pragma once

#include "cgfx.pb.h" // Proto-generated error codes
#include <string>
#include <ctime>

namespace cryo {

// Use proto-generated CryoErrorCode enum from cgfx.proto
// This allows error codes to be shared across C++, TypeScript, Python, etc.
using CryoErrorCode = cgfx::CryoErrorCode;

// Global error code tracking (accessible from coregfx modules)
extern CryoErrorCode g_cryoErrorCode;

// Set error code before potentially crashing operations
inline void setCryoErrorCode(CryoErrorCode code) {
    g_cryoErrorCode = code;
}

// Get current error code
inline CryoErrorCode getCryoErrorCode() {
    return g_cryoErrorCode;
}

// Convert CryoErrorCode to human-readable string
inline const char* cryoErrorCodeToString(CryoErrorCode code) {
    switch (code) {
        case cgfx::CRYO_SUCCESS: return "Success";
        case cgfx::CRYO_ERROR_VULKAN_INSTANCE_FAILED: return "Vulkan instance creation failed";
        case cgfx::CRYO_ERROR_VULKAN_DEVICE_FAILED: return "Vulkan device selection/creation failed";
        case cgfx::CRYO_ERROR_VULKAN_SWAPCHAIN_FAILED: return "Vulkan swapchain creation failed";
        case cgfx::CRYO_ERROR_VULKAN_RENDERPASS_FAILED: return "Vulkan renderpass creation failed";
        case cgfx::CRYO_ERROR_VULKAN_FENCE_FAILED: return "Vulkan fence creation failed";
        case cgfx::CRYO_ERROR_VULKAN_SEMAPHORE_FAILED: return "Vulkan semaphore creation failed";
        case cgfx::CRYO_ERROR_VULKAN_COMMAND_BUFFER_FAILED: return "Vulkan command buffer allocation failed";
        case cgfx::CRYO_ERROR_VULKAN_DESCRIPTOR_POOL_FAILED: return "Vulkan descriptor pool creation failed";
        case cgfx::CRYO_ERROR_VULKAN_DESCRIPTOR_SET_FAILED: return "Vulkan descriptor set allocation failed";
        case cgfx::CRYO_ERROR_VULKAN_PIPELINE_FAILED: return "Vulkan pipeline creation failed";
        case cgfx::CRYO_ERROR_VULKAN_BUFFER_FAILED: return "Vulkan buffer creation failed";
        case cgfx::CRYO_ERROR_VULKAN_TEXTURE_FAILED: return "Vulkan texture loading failed";
        case cgfx::CRYO_ERROR_GLTF_LOAD_FAILED: return "GLTF model loading failed";
        case cgfx::CRYO_ERROR_GLTF_PARSE_FAILED: return "GLTF file parsing failed";
        case cgfx::CRYO_ERROR_ASSET_PATH_NOT_FOUND: return "Asset path not found";
        case cgfx::CRYO_ERROR_ENVIRONMENT_MAP_FAILED: return "Environment map loading failed";
        case cgfx::CRYO_ERROR_BRDF_LUT_FAILED: return "BRDF LUT generation failed";
        case cgfx::CRYO_ERROR_CUBEMAP_FAILED: return "Cubemap generation failed";
        case cgfx::CRYO_ERROR_RUNTIME_EXCEPTION: return "Runtime exception";
        case cgfx::CRYO_ERROR_UNKNOWN_EXCEPTION: return "Unknown exception";
        case cgfx::CRYO_ERROR_FATAL_CRASH: return "Fatal crash";
        default: return "Unknown Cryo error";
    }
}

// Convert VkResult to human-readable string
inline const char* vkResultToString(int32_t vkResult) {
    switch (vkResult) {
        case 0: return "VK_SUCCESS";
        case 1: return "VK_NOT_READY";
        case 2: return "VK_TIMEOUT";
        case 3: return "VK_EVENT_SET";
        case 4: return "VK_EVENT_RESET";
        case 5: return "VK_INCOMPLETE";
        case -1: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case -2: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case -3: return "VK_ERROR_INITIALIZATION_FAILED";
        case -4: return "VK_ERROR_DEVICE_LOST";
        case -5: return "VK_ERROR_MEMORY_MAP_FAILED";
        case -6: return "VK_ERROR_LAYER_NOT_PRESENT";
        case -7: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case -8: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case -9: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case -10: return "VK_ERROR_TOO_MANY_OBJECTS";
        case -11: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case -12: return "VK_ERROR_FRAGMENTED_POOL";
        case -13: return "VK_ERROR_UNKNOWN";
        case -1000069000: return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case -1000072003: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case -1000161000: return "VK_ERROR_FRAGMENTATION";
        case -1000257000: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case 1000297000: return "VK_PIPELINE_COMPILE_REQUIRED";
        case -1000000000: return "VK_ERROR_SURFACE_LOST_KHR";
        case -1000000001: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case 1000001003: return "VK_SUBOPTIMAL_KHR";
        case -1000001004: return "VK_ERROR_OUT_OF_DATE_KHR";
        case -1000003001: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case -1000011001: return "VK_ERROR_VALIDATION_FAILED_EXT";
        case -1000012000: return "VK_ERROR_INVALID_SHADER_NV";
        case -1000158000: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case -1000174001: return "VK_ERROR_NOT_PERMITTED_EXT";
        case -1000255000: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case 1000268000: return "VK_THREAD_IDLE_KHR";
        case 1000268001: return "VK_THREAD_DONE_KHR";
        case 1000268002: return "VK_OPERATION_DEFERRED_KHR";
        case 1000268003: return "VK_OPERATION_NOT_DEFERRED_KHR";
        default: return "VK_UNKNOWN_RESULT";
    }
}

// Create ErrorReport message with VkResult info
inline cgfx::ErrorReport createErrorReport(CryoErrorCode code, int32_t vkResult, const std::string& details, const std::string& rendererType = "unknown") {
    cgfx::ErrorReport report;
    report.set_code(code);
    report.set_message(cryoErrorCodeToString(code));
    report.set_details(details);
    report.set_renderer_id(rendererType);
    report.set_vk_result(vkResult);
    report.set_vk_result_name(vkResultToString(vkResult));
    report.set_timestamp(std::time(nullptr));
    return report;
}

// Helper: Set error code and create detailed error message with VkResult
inline void setCryoErrorCodeWithVkResult(CryoErrorCode code, int32_t vkResult, const std::string& details = "") {
    setCryoErrorCode(code);
    // Error report can be retrieved later via getCryoErrorCode() and vkResult
}

} // namespace cryo