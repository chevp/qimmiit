/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef OCEAN_DEPENDENCIES_HPP_
#define OCEAN_DEPENDENCIES_HPP_

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <fcntl.h>
#include <io.h>
// #include <atlstr.h> // Not needed - CString not used
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>
// #include "VulkanAndroid.h"
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <wayland-client.h>
#elif defined(_DIRECT2DISPLAY)
//
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#include <QuartzCore/CAMetalLayer.h>
#include <CoreVideo/CVDisplayLink.h>
#endif

#include <iostream>
#include <chrono>
#include <sys/stat.h>
#include <map>
#include <string>
#include <string_view>

///////////////////////////
// 2023-06-24
// #define GLEW_STATIC // This must be defined when statically linking to GLEW
// #include "glew/glew.h"
// #pragma comment (lib, "glew32s.lib") // Link to glew32s.lib => no need for glew32.dll
///////////////////////////////
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <sstream>
#include <array>
#include <numeric>
#include <exception>
#include <algorithm>
#include <cstring>
#include <vector>
#include <map>

#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
#include <vulkan/vulkan_beta.h>
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "VulkanAndroid.h"
#endif

#include <stdexcept>
#include <iostream>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gli/image.hpp>
#include <gli/texture2d.hpp>
#include <gli/load.hpp>
#include <gli/texture_cube.hpp>

#ifndef THREAD_POOL_HPP_
#include <coregfx/util/thread_pool.hpp>
#endif

#ifndef UTIL_HPP_
#include <coregfx/util/util.hpp>
#endif

#ifndef OCEAN_LOG_HPP_
#include <coregfx/core/ocean_log.hpp>
#endif

// Replace standard assert with exception-throwing version (no annoying popups)
#ifdef assert
#undef assert
#endif

#ifdef _DEBUG
#define assert(condition) \
    do { \
        if (!(condition)) { \
            std::string assertMsg = std::string("ASSERTION FAILED: " #condition " at ") + __FILE__ + ":" + std::to_string(__LINE__); \
            ocean::error(assertMsg); \
            throw std::runtime_error("Assertion failed: " #condition); \
        } \
    } while(0)
#else
#define assert(condition) ((void)0)
#endif

#endif