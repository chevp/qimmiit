# ==============================================================================
# cryo-render-cli - Headless Vulkan Asset Renderer
# Copyright (c) 2025 Patrice Chevillat
# ==============================================================================

cmake_minimum_required(VERSION 3.20)
project(cryo-render-cli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Dependencies
# ==============================================================================

find_package(Vulkan REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

# stb_image_write for PNG encoding (header-only)
find_path(STB_INCLUDE_DIRS "stb_image_write.h")

# ==============================================================================
# Library: cryo_render (Asset Generation Pipeline)
# ==============================================================================

add_library(cryo_render STATIC
    src/AssetGenerationPipeline.cpp
    src/FramebufferCapture.cpp
    src/HeadlessRenderer.cpp
    src/RenderJobProcessor.cpp
    src/SpriteSheetGenerator.cpp
    src/TileSetRenderer.cpp
)

target_include_directories(cryo_render
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${STB_INCLUDE_DIRS}
)

target_link_libraries(cryo_render
    PUBLIC
        Vulkan::Vulkan
        nlohmann_json::nlohmann_json
        protobuf::libprotobuf
    PRIVATE
        coregfx          # Link to existing Vulkan utilities
        cryo_asset       # Asset loading
)

target_compile_definitions(cryo_render
    PRIVATE
        CRYO_RENDER_VERSION="${PROJECT_VERSION}"
)

# ==============================================================================
# Executable: cryo-render-cli
# ==============================================================================

add_executable(cryo-render-cli
    src/main.cpp
    src/cli/CommandLineParser.cpp
    src/cli/RenderCommands.cpp
    src/cli/BatchCommands.cpp
)

target_link_libraries(cryo-render-cli
    PRIVATE
        cryo_render
        nlohmann_json::nlohmann_json
)

# ==============================================================================
# Installation
# ==============================================================================

install(TARGETS cryo-render-cli
    RUNTIME DESTINATION bin
)

install(TARGETS cryo_render
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

# ==============================================================================
# Examples
# ==============================================================================

add_subdirectory(examples)