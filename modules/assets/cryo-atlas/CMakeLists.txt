# ==============================================================================
# cryo-atlas - Texture Atlas Packer
# Copyright (c) 2025 Patrice Chevillat
# ==============================================================================

cmake_minimum_required(VERSION 3.20)
project(cryo-atlas VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Dependencies
# ==============================================================================

find_package(nlohmann_json CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

# stb_image for reading PNGs
find_path(STB_INCLUDE_DIRS "stb_image.h")

# ==============================================================================
# Library: cryo_atlas
# ==============================================================================

add_library(cryo_atlas STATIC
    src/AtlasPacker.cpp
    src/RectanglePacker.cpp
    src/AtlasGenerator.cpp
    src/MetadataExporter.cpp
)

target_include_directories(cryo_atlas
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${STB_INCLUDE_DIRS}
)

target_link_libraries(cryo_atlas
    PUBLIC
        nlohmann_json::nlohmann_json
        protobuf::libprotobuf
)

target_compile_definitions(cryo_atlas
    PRIVATE
        CRYO_ATLAS_VERSION="${PROJECT_VERSION}"
)

# ==============================================================================
# Executable: cryo-atlas-cli
# ==============================================================================

add_executable(cryo-atlas-cli
    src/main.cpp
)

target_link_libraries(cryo-atlas-cli
    PRIVATE
        cryo_atlas
        nlohmann_json::nlohmann_json
)

# ==============================================================================
# Installation
# ==============================================================================

install(TARGETS cryo-atlas-cli
    RUNTIME DESTINATION bin
)

install(TARGETS cryo_atlas
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)