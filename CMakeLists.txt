cmake_minimum_required(VERSION 3.20)

project(arctic-game-client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect package management mode
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    set(USING_VCPKG TRUE)
else()
    message(STATUS "Using system package manager")
    set(USING_VCPKG FALSE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform-specific configuration
if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    add_definitions(-DKTX_OPENGL)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DSOLUTION_NAME="${PROJECT_NAME}")

    # Disable .pdb generation in Release builds to reduce artifact size
    if(MSVC)
        string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/DEBUG" "" CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
        string(REPLACE "/DEBUG" "" CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
    endif()
elseif(APPLE)
    add_definitions(-DVK_USE_PLATFORM_METAL_EXT)
    # MoltenVK for Vulkan on macOS
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")
elseif(UNIX)
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    find_package(Threads REQUIRED)
endif()

# Disable excessive logging in Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG -DNO_VERBOSE_LOGGING)
endif()

# Find required packages
find_package(Vulkan REQUIRED)

# Flexible package finding (works with both vcpkg and system packages)
if(USING_VCPKG)
    # vcpkg prefers CONFIG mode
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_package(Protobuf CONFIG REQUIRED)
    find_package(gRPC CONFIG REQUIRED)
    find_package(tinyxml2 CONFIG REQUIRED)
    find_package(glew CONFIG REQUIRED)
    find_package(tinygltf CONFIG QUIET)
else()
    # System package managers use MODULE mode
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(Protobuf REQUIRED)
    find_package(gRPC REQUIRED)

    # tinyxml2 may need CONFIG on some systems
    find_package(tinyxml2 CONFIG QUIET)
    if(NOT tinyxml2_FOUND)
        find_package(tinyxml2 REQUIRED)
    endif()

    find_package(GLEW REQUIRED)
    find_package(tinygltf CONFIG QUIET)
endif()

# tinygltf fallback for bundled version
if(NOT tinygltf_FOUND)
    message(STATUS "tinygltf not found, using bundled version if available")
endif()

# =============================================================================
# Project Structure
# =============================================================================
# The project is now organized as:
#   - modules/        : Core engine modules
#   - apps/           : Standalone applications
#   - orchestrators/  : Orchestration systems
#   - content-tools/  : Content creation tools
#   - sdk/            : SDK for developers

# =============================================================================
# Add arctic-formats (required by many modules)
# =============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/arctic-formats/CMakeLists.txt")
    add_subdirectory(arctic-formats)
else()
    message(WARNING "arctic-formats not found - some modules may fail")
endif()

# =============================================================================
# Add Modules (from modules/ directory)
# =============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/CMakeLists.txt")
    message(STATUS "Adding modules from modules/")
    add_subdirectory(modules)
else()
    message(WARNING "modules/ directory not found")

    # Fallback: Try to add individual modules if they exist
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/rendering/coregfx/CMakeLists.txt")
        add_subdirectory(modules/rendering/coregfx)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/assets/cryo-cache/CMakeLists.txt")
        add_subdirectory(modules/assets/cryo-cache)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/assets/cryo-asset/CMakeLists.txt")
        add_subdirectory(modules/assets/cryo-asset)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/toolings/akutik/CMakeLists.txt")
        add_subdirectory(modules/toolings/akutik)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/toolings/cryo-tooling/CMakeLists.txt")
        add_subdirectory(modules/toolings/cryo-tooling)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/rendering/data-driven-renderer/CMakeLists.txt")
        add_subdirectory(modules/rendering/data-driven-renderer)
    endif()
endif()

# =============================================================================
# Add Orchestrators (must come before apps that depend on them)
# =============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/orchestrators/CMakeLists.txt")
    message(STATUS "Adding orchestrators")
    add_subdirectory(orchestrators)
endif()

# =============================================================================
# Add Applications (from apps/ directory)
# =============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/apps/CMakeLists.txt")
    message(STATUS "Adding applications from apps/")
    add_subdirectory(apps)
endif()

# =============================================================================
# Add Content Tools (Optional)
# =============================================================================
option(BUILD_CONTENT_TOOLS "Build content creation tools" OFF)
if(BUILD_CONTENT_TOOLS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/content-tools/CMakeLists.txt")
    message(STATUS "Adding content tools")
    add_subdirectory(content-tools)
endif()