/*
 * Global macros
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once

#ifndef OCEAN_MACROS_HPP_
#define OCEAN_MACROS_HPP_

#ifndef OCEAN_DEPENDENCIES_HPP_
#include "ocean_dependencies.hpp"
#endif

#include "ocean_log.hpp"

// Helper function to convert VkResult to string
inline const char* vkResultToString(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
        default: return "UNKNOWN_VK_RESULT";
    }
}

#if defined(__ANDROID__)
#define VK_CHECK_RESULT(f)                                                                  \
	{                                                                                       \
		VkResult res = (f);                                                                 \
		if (res != VK_SUCCESS)                                                              \
		{                                                                                   \
			std::string errorMsg = "VULKAN ERROR: " + std::string(vkResultToString(res)) + " (" + std::to_string(res) + ") in " + __FILE__ + " at line " + std::to_string(__LINE__) + " - Function: " + #f; \
			LOGE("%s", errorMsg.c_str()); \
			ocean::error(errorMsg); \
			std::exit(res);                                                                 \
		}                                                                                   \
	}
#else
#define VK_CHECK_RESULT(f)                                                                                                \
	{                                                                                                                     \
		VkResult res = (f);                                                                                               \
		if (res != VK_SUCCESS)                                                                                            \
		{                                                                                                                 \
			std::string errorMsg = "VULKAN ERROR: " + std::string(vkResultToString(res)) + " (" + std::to_string(res) + ") in " + __FILE__ + " at line " + std::to_string(__LINE__) + " - Function: " + #f; \
			ocean::error(errorMsg); \
			std::cout << errorMsg << std::endl; \
			std::exit(res);                                                                                               \
		}                                                                                                                 \
	}
#endif

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                                              \
	{                                                                                                         \
		fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetInstanceProcAddr(inst, "vk" #entrypoint)); \
		if (fp##entrypoint == NULL)                                                                           \
		{                                                                                                     \
			exit(1);                                                                                          \
		}                                                                                                     \
	}

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                                              \
	{                                                                                                      \
		fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetDeviceProcAddr(dev, "vk" #entrypoint)); \
		if (fp##entrypoint == NULL)                                                                        \
		{                                                                                                  \
			exit(1);                                                                                       \
		}                                                                                                  \
	}

#endif