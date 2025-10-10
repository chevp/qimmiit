# CoreGFX Graphics Library
cmake_minimum_required(VERSION 3.20)

project(coregfx VERSION 1.0.0 LANGUAGES CXX)

# Library type option
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# ============================================================================
# Proto Generation (coregfx owns cgfx.proto, frostapi.proto, etc.)
# ============================================================================

set(COREGFX_PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(COREGFX_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/cpp")

# Create generated directory
file(MAKE_DIRECTORY ${COREGFX_GENERATED_DIR})

# Proto files
file(GLOB COREGFX_PROTO_FILES "${COREGFX_PROTO_DIR}/*.proto")

# Get protoc and grpc plugin paths (from parent scope)
if(NOT PROTOBUF_PROTOC_EXECUTABLE)
    get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
    if(NOT PROTOBUF_PROTOC_EXECUTABLE)
        get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_DEBUG)
    endif()
endif()

if(NOT GRPC_CPP_PLUGIN)
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin
        PATHS ${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/grpc
        NO_DEFAULT_PATH
    )
endif()

# Generate C++ files from proto files
foreach(PROTO_FILE ${COREGFX_PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    set(PROTO_SRC "${COREGFX_GENERATED_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${COREGFX_GENERATED_DIR}/${PROTO_NAME}.pb.h")
    set(GRPC_SRC "${COREGFX_GENERATED_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDR "${COREGFX_GENERATED_DIR}/${PROTO_NAME}.grpc.pb.h")

    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR} ${GRPC_SRC} ${GRPC_HDR}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --proto_path=${COREGFX_PROTO_DIR}
             --cpp_out=${COREGFX_GENERATED_DIR}
             --grpc_out=${COREGFX_GENERATED_DIR}
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
             ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating C++ code from ${PROTO_NAME}.proto"
        VERBATIM
    )

    list(APPEND COREGFX_GENERATED_SOURCES ${PROTO_SRC} ${GRPC_SRC})
    list(APPEND COREGFX_GENERATED_HEADERS ${PROTO_HDR} ${GRPC_HDR})
endforeach()

message(STATUS "CoreGFX Proto configured:")
message(STATUS "  Proto dir: ${COREGFX_PROTO_DIR}")
message(STATUS "  Generated: ${COREGFX_GENERATED_DIR}")
message(STATUS "  Proto files: ${COREGFX_PROTO_FILES}")

# Create the coregfx library
if(BUILD_SHARED_LIBS)
    add_library(coregfx SHARED)
    target_compile_definitions(coregfx PRIVATE COREGFX_EXPORTS)
    target_compile_definitions(coregfx PUBLIC COREGFX_DLL)
else()
    add_library(coregfx STATIC)
    target_compile_definitions(coregfx PUBLIC COREGFX_STATIC)
endif()

# Source files based on vcxproj analysis
target_sources(coregfx PRIVATE
    # Main Sources
    src/vulkan_context.cpp

    # Core Module Sources
    src/core/ocean_base.cpp
    src/core/ocean_core.cpp
    src/core/ocean_gltf.cpp
    src/core/ocean_log.cpp
    src/core/ocean_swap_chain.cpp
    src/core/ocean_texture.cpp
    src/core/ocean_tools.cpp
    src/core/ocean_util.cpp
    src/core/ConfigProvider.cpp
    src/core/ConfigManager.cpp
    src/core/CryoErrorCodes.cpp

    # GLTF Module Sources
    src/gltf/gltf_service.cpp

    # RSC Module Sources
    src/rsc/rsc.cpp
    src/rsc/AssetLoader.cpp

    # Util Module Sources
    src/util/CommandLineParser.cpp
    src/util/input_handler.cpp
    src/util/vulkan_initializer.cpp
    src/util/framebuffer_manager.cpp
    src/util/command_buffer_manager.cpp
    src/util/synchronization_manager.cpp
    src/util/gpu_selector.cpp
    src/util/window_resize_manager.cpp
    src/util/platform/windows_platform.cpp
    src/util/vulkan_state_logger.cpp
    src/util/compact_logger.cpp
    src/util/json_event_logger.cpp

    # External Module Sources
    adapters/imgui_stubs.cpp
    ${CMAKE_SOURCE_DIR}/external/header-only/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/header-only/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/external/header-only/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/header-only/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/header-only/tinygltf/tiny_gltf.cc

    # Overlay Module Sources
    src/overlay/overlay_imgui.cpp
    src/overlay/tool_launcher.cpp

    # PBR App Module Sources
    src/pbr/pbr_app.cpp

    # Protobuf Generated Sources (coregfx proto files)
    ${COREGFX_GENERATED_SOURCES}
)

# Public headers
target_sources(coregfx PUBLIC FILE_SET HEADERS
    BASE_DIRS include
    FILES
        # Main Headers
        include/coregfx/coregfx_export.hpp
        include/coregfx/ocean.hpp
        include/coregfx/vulkan_context.hpp
        include/coregfx/vulkan_renderer.hpp

        # Core Module Headers
        include/coregfx/core/ocean.hpp
        include/coregfx/core/ocean_base.hpp
        include/coregfx/core/ocean_config.hpp
        include/coregfx/core/ocean_core.hpp
        include/coregfx/core/ocean_dependencies.hpp
        include/coregfx/core/ocean_gltf.hpp
        include/coregfx/core/ocean_initializer.hpp
        include/coregfx/core/ocean_keycodes.hpp
        include/coregfx/core/ocean_log.hpp
        include/coregfx/core/ocean_macros.hpp
        include/coregfx/core/ocean_swap_chain.hpp
        include/coregfx/core/ocean_texture.hpp
        include/coregfx/core/ocean_tools.hpp
        include/coregfx/core/ocean_util.hpp
        include/coregfx/core/ConfigProvider.hpp
        include/coregfx/core/ConfigManager.hpp
        include/coregfx/core/RendererConstants.hpp

        # GLTF Module Headers
        include/coregfx/gltf/gltf_folder.hpp
        include/coregfx/gltf/gltf_forward_decls.hpp
        include/coregfx/gltf/gltf_loader.hpp
        include/coregfx/gltf/gltf_modifier.hpp
        include/coregfx/gltf/gltf_service.hpp
        include/coregfx/gltf/gltf_sqlite_repo.hpp

        # RSC Module Headers
        include/coregfx/rsc/AssetLoader.hpp
        include/coregfx/rsc/json_elyrion.hpp
        include/coregfx/rsc/rsc.hpp
        include/coregfx/rsc/rsc_manager.hpp
        include/coregfx/rsc/rsc_pocean.hpp
        include/coregfx/rsc/xml_elyrion.hpp
        include/coregfx/rsc/xml_pocean.hpp

        # Overlay Module Headers
        include/coregfx/overlay/overlay.hpp
        include/coregfx/overlay/overlay_imgui.hpp
        include/coregfx/overlay/tool_launcher.hpp
        include/coregfx/overlay/tool_menu_integration.hpp

        # PBR App Module Headers
        include/coregfx/pbr/pbr.hpp
        include/coregfx/pbr/pbr_app.hpp

        # Util Module Headers
        include/coregfx/util/LunaraUtil.hpp
        include/coregfx/util/CommandLineParser.hpp
        include/coregfx/util/thread_pool.hpp
        include/coregfx/util/util.hpp
        include/coregfx/util/input_handler.hpp
        include/coregfx/util/vulkan_initializer.hpp
        include/coregfx/util/framebuffer_manager.hpp
        include/coregfx/util/command_buffer_manager.hpp
        include/coregfx/util/synchronization_manager.hpp
        include/coregfx/util/gpu_selector.hpp
        include/coregfx/util/window_resize_manager.hpp
        include/coregfx/util/platform/windows_platform.hpp
        include/coregfx/util/vulkan_state_logger.hpp
        include/coregfx/util/compact_logger.hpp
        include/coregfx/util/json_event_logger.hpp
)

# Include directories
target_include_directories(coregfx
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/header-only/tinygltf>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/header-only/imgui>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/header-only>
        $<BUILD_INTERFACE:${COREGFX_GENERATED_DIR}>  # coregfx proto headers
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/arctic-formats/generated/cpp>  # arctic-formats proto headers
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external/prebuilt/include
)

# Link dependencies
target_link_libraries(coregfx
    PUBLIC
        Vulkan::Vulkan
        glm::glm

        # Protobuf - used for LOCAL data serialization only (config, cache)
        protobuf::libprotobuf

        # ⚠️ ARCHITECTURAL WARNING - gRPC dependencies should be removed
        # These are linked due to legacy ipcoe17_client.cpp code
        # TODO: Move ipcoe17_client.cpp to apps/foundation/coregfx-stream-client/
        # TODO: Remove these gRPC dependencies after migration
        # REFERENCE: See coregfx/ARCHITECTURE_ANALYSIS.md
        gRPC::gpr      # ⚠️ WARNING: Should be removed
        gRPC::grpc     # ⚠️ WARNING: Should be removed
        gRPC::grpc++   # ⚠️ WARNING: Should be removed

        tinyxml2::tinyxml2
        GLEW::GLEW
    PRIVATE
        $<$<PLATFORM_ID:Windows>:opengl32>
        $<$<PLATFORM_ID:Windows>:ws2_32>
        $<$<PLATFORM_ID:Windows>:crypt32>
)

# Platform-specific settings
if(WIN32)
    # Windows-specific preprocessor definitions
    target_compile_definitions(coregfx PRIVATE
        WIN32_LEAN_AND_MEAN
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )

    # Disable specific warnings on Windows
    if(MSVC)
        target_compile_options(coregfx PRIVATE
            /FS      # Fix parallel compilation PDB conflicts
            /wd4251  # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
            /wd4275  # non dll-interface class 'type1' used as base for dll-interface class 'type2'
            /wd4996  # 'function': was declared deprecated
            /wd4244  # 'conversion' conversion from 'type1' to 'type2', possible loss of data
            /wd4267  # 'var' : conversion from 'size_t' to 'type', possible loss of data
            /wd4100  # 'identifier' : unreferenced formal parameter
            /wd4127  # conditional expression is constant
            /wd4125  # decimal digit terminates octal escape sequence
            /wd4146  # unary minus operator applied to unsigned type, result still unsigned
            /wd4800  # 'type' : forcing value to bool 'true' or 'false' (performance warning)
        )
    endif()
endif()

# Export targets for downstream projects
install(TARGETS coregfx
    EXPORT coregfxTargets
    FILE_SET HEADERS
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(EXPORT coregfxTargets
    FILE coregfxTargets.cmake
    NAMESPACE coregfx::
    DESTINATION lib/cmake/coregfx
)

# Generate and install config files
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/coregfxConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/coregfxConfig.cmake"
    INSTALL_DESTINATION lib/cmake/coregfx
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/coregfxConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/coregfxConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/coregfxConfigVersion.cmake"
    DESTINATION lib/cmake/coregfx
)