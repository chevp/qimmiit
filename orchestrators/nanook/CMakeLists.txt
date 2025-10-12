cmake_minimum_required(VERSION 3.20)

project(nanook VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_NANOOK_CLI "Build nanook-cli executable" ON)
option(BUILD_NANOOK_SHARED "Build nanook as shared library" OFF)

# Find dependencies for Nanook
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

# ============================================================================
# Core Engine Library (static) - Standalone, no external dependencies
# ============================================================================

set(NANOOK_CORE_SOURCES
    src/core/CentralAgent.cpp
    src/core/ConfigManager.cpp
    src/core/LogManager.cpp
    src/core/EventBus.cpp
    src/core/TaskScheduler.cpp
    src/core/MemoryManager.cpp
    src/core/SpecificationGenerator.cpp
)

set(NANOOK_CORE_HEADERS
    src/core/CentralAgent.hpp
    src/core/IModule.hpp
    src/core/ModuleSpec.hpp
    src/core/ConfigManager.hpp
    src/core/LogManager.hpp
    src/core/EventBus.hpp
    src/core/TaskScheduler.hpp
    src/core/MemoryManager.hpp
    src/core/SpecificationGenerator.hpp
    src/core/EngineConfig.hpp
    src/core/Types.hpp
)

add_library(nanook_core STATIC
    ${NANOOK_CORE_SOURCES}
)

target_include_directories(nanook_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================================================
# Module Implementations (static libraries)
# ============================================================================

add_library(nanook_renderer_module STATIC
    src/modules/RendererModule.cpp
    src/modules/RendererModule.hpp
)
target_link_libraries(nanook_renderer_module
    PUBLIC
        nanook_core
        glfw
        Vulkan::Vulkan
)

add_library(nanook_cache_module STATIC
    src/modules/CacheModule.cpp
    src/modules/CacheModule.hpp
)
target_link_libraries(nanook_cache_module PUBLIC nanook_core)

add_library(nanook_asset_module STATIC
    src/modules/AssetModule.cpp
    src/modules/AssetModule.hpp
)
target_link_libraries(nanook_asset_module PUBLIC nanook_core)

add_library(nanook_http_module STATIC
    src/modules/HttpModule.cpp
    src/modules/HttpModule.hpp
)
target_link_libraries(nanook_http_module PUBLIC nanook_core)

add_library(nanook_compute_module STATIC
    src/modules/ComputeModule.cpp
    src/modules/ComputeModule.hpp
)
target_link_libraries(nanook_compute_module PUBLIC nanook_core)

# ============================================================================
# Nanook Complete Library (combines core + all modules)
# ============================================================================

add_library(nanook STATIC
    ${NANOOK_CORE_SOURCES}
    src/modules/RendererModule.cpp
    src/modules/CacheModule.cpp
    src/modules/AssetModule.cpp
    src/modules/HttpModule.cpp
    src/modules/ComputeModule.cpp
    src/Nanook.cpp
)

target_include_directories(nanook
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(nanook
    PUBLIC
        glfw
        Vulkan::Vulkan
)

# Optional: Build as shared library
if(BUILD_NANOOK_SHARED)
    add_library(nanook_shared SHARED
        ${NANOOK_CORE_SOURCES}
        src/modules/RendererModule.cpp
        src/modules/CacheModule.cpp
        src/modules/AssetModule.cpp
        src/modules/HttpModule.cpp
        src/modules/ComputeModule.cpp
    )

    target_include_directories(nanook_shared
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    set_target_properties(nanook_shared PROPERTIES
        OUTPUT_NAME nanook
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )
endif()

# ============================================================================
# Nanook CLI Executable (optional)
# ============================================================================

if(BUILD_NANOOK_CLI)
    add_executable(nanook-cli
        src/cli/main_simple.cpp
    )

    target_link_libraries(nanook-cli
        PRIVATE
            nanook  # Link against the complete nanook library
    )
endif()

# ============================================================================
# Installation
# ============================================================================

install(TARGETS nanook nanook_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY src/core/
    DESTINATION include/nanook/core
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY src/modules/
    DESTINATION include/nanook/modules
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY include/nanook/
    DESTINATION include/nanook
    FILES_MATCHING PATTERN "*.hpp"
)

if(BUILD_NANOOK_CLI)
    install(TARGETS nanook-cli
        RUNTIME DESTINATION bin
    )
endif()

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "===== Nanook Engine Configuration =====")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build CLI: ${BUILD_NANOOK_CLI}")
message(STATUS "Build Shared Library: ${BUILD_NANOOK_SHARED}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Libraries to be built:")
message(STATUS "  - nanook.lib (complete static library)")
message(STATUS "  - nanook_core.lib (core only)")
message(STATUS "  - nanook_*_module.lib (individual modules)")
if(BUILD_NANOOK_SHARED)
    message(STATUS "  - nanook.dll/.so (shared library)")
endif()
if(BUILD_NANOOK_CLI)
    message(STATUS "  - nanook-cli.exe (CLI executable)")
endif()
message(STATUS "======================================")
message(STATUS "")
