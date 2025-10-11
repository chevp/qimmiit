# Data-Driven CoreGFX Renderer Application
cmake_minimum_required(VERSION 3.20)

project(data_driven_coregfx_renderer VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# Use Shared Arctic-Proto Generation from Root CMakeLists.txt
# ============================================================================

# Arctic-proto files are generated at root level: generated/cpp/
# Variables available from parent scope:
#   - ARCTIC_PROTO_OUTPUT_DIR: ${CMAKE_SOURCE_DIR}/generated/cpp
#   - ARCTIC_GENERATED_PROTO_SOURCES: List of generated .pb.cc and .grpc.pb.cc files
#   - ARCTIC_GENERATED_PROTO_HEADERS: List of generated .pb.h and .grpc.pb.h files

message(STATUS "Data-driven renderer using shared arctic-proto generation:")
message(STATUS "  Output: ${ARCTIC_PROTO_OUTPUT_DIR}")

# ============================================================================
# Create the executable
# ============================================================================

add_executable(data_driven_coregfx_renderer)

# Source files
target_sources(data_driven_coregfx_renderer PRIVATE
    src/main.cpp
    src/ConfigParser.cpp
    src/ShaderManager.cpp
    src/Renderer.cpp
    src/GrpcRendererClient.cpp
    src/MaterialOverride.cpp

    # NOTE: Proto files are compiled in coregfx library, not here
    # NOTE: AssetLoader moved to coregfx/rsc/AssetLoader
)

# Headers
target_sources(data_driven_coregfx_renderer PRIVATE
    src/Config.hpp
    src/ConfigParser.hpp
    src/ShaderManager.hpp
    src/Renderer.hpp
    src/GrpcRendererClient.hpp
    src/GeometryFactory.hpp
    src/MaterialOverride.hpp

    # NOTE: Proto headers are accessible via coregfx include paths
    # NOTE: AssetLoader.hpp moved to coregfx/rsc/AssetLoader.hpp
)

# Include directories
target_include_directories(data_driven_coregfx_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/arctic-formats/generated/cpp  # arctic-formats proto headers
    ${CMAKE_SOURCE_DIR}/coregfx/generated/cpp  # coregfx proto headers
    ${CMAKE_SOURCE_DIR}/external/header-only
)

# Proto generation is now handled by arctic-formats library
# No explicit dependency needed - linking against arctic-formats is sufficient

# Link against libraries (includes proto files)
target_link_libraries(data_driven_coregfx_renderer PRIVATE
    coregfx
    arctic-formats  # Arctic data formats (arctic_*.proto)
    cryo_tooling    # Cryo tooling (cryo_*.proto)
    protobuf::libprotobuf
    gRPC::gpr
    gRPC::grpc
    gRPC::grpc++
    tinyxml2::tinyxml2
    Vulkan::Vulkan
    glm::glm
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(data_driven_coregfx_renderer PRIVATE
        ws2_32
        crypt32
    )
endif()

# Set target properties
set_target_properties(data_driven_coregfx_renderer PROPERTIES
    OUTPUT_NAME "data-driven-coregfx-renderer"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Platform-specific settings
if(WIN32)
    set_target_properties(data_driven_coregfx_renderer PROPERTIES
        WIN32_EXECUTABLE ON  # Win32 GUI application (uses WinMain)
    )

    target_compile_definitions(data_driven_coregfx_renderer PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        _CONSOLE
    )

    # Disable specific warnings
    if(MSVC)
        target_compile_options(data_driven_coregfx_renderer PRIVATE
            /EHa     # Enable SEH and C++ exception handling
            /wd4251  # dll-interface warnings
            /wd4275
            /wd4996  # deprecated warnings
            /wd4244  # conversion warnings
            /wd4267
            /wd4100  # unreferenced parameter
            /wd4127  # conditional expression is constant
        )
    endif()
endif()

# Copy scene files, config, and assets to build directory
add_custom_command(TARGET data_driven_coregfx_renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/data-driven.index.xml
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/data-driven.index.xml
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/data-driven.index.xml
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/index.arctic
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/arctic.config.xml
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/arctic.config.xml
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/assets
    COMMENT "Copying scene files, root config, and assets to build directory"
)

# Install target
install(TARGETS data_driven_coregfx_renderer
    RUNTIME DESTINATION bin
)
