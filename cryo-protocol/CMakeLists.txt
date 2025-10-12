cmake_minimum_required(VERSION 3.20)
project(cryo-protocol VERSION 1.0.0 LANGUAGES CXX)

# Find required packages
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# ============================================================================
# Proto Generation (Cryo Protocol owns its proto files)
# ============================================================================

set(CRYO_PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(CRYO_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/cpp")

# Create generated directory if it doesn't exist
file(MAKE_DIRECTORY ${CRYO_GENERATED_DIR})

# Proto files - collect from all subdirectories
file(GLOB_RECURSE CRYO_PROTO_FILES
    "${CRYO_PROTO_DIR}/*.proto"
)

# Get protoc and grpc plugin paths
get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
if(NOT PROTOBUF_PROTOC_EXECUTABLE)
    get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_DEBUG)
endif()

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin
    PATHS ${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/grpc
    NO_DEFAULT_PATH
)

# Generate C++ files from proto files
set(GENERATED_SOURCES)
set(GENERATED_HEADERS)

foreach(PROTO_FILE ${CRYO_PROTO_FILES})
    # Get relative path to maintain directory structure
    file(RELATIVE_PATH PROTO_REL_PATH ${CRYO_PROTO_DIR} ${PROTO_FILE})
    get_filename_component(PROTO_DIR ${PROTO_REL_PATH} DIRECTORY)
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    # Create output directory structure
    set(OUTPUT_DIR "${CRYO_GENERATED_DIR}/${PROTO_DIR}")
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    # Define output files
    set(PROTO_SRC "${OUTPUT_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${OUTPUT_DIR}/${PROTO_NAME}.pb.h")
    set(GRPC_SRC "${OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDR "${OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h")

    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR} ${GRPC_SRC} ${GRPC_HDR}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --proto_path=${CRYO_PROTO_DIR}
             --cpp_out=${CRYO_GENERATED_DIR}
             --grpc_out=${CRYO_GENERATED_DIR}
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
             ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating C++ code from ${PROTO_REL_PATH}"
        VERBATIM
    )

    list(APPEND GENERATED_SOURCES ${PROTO_SRC} ${GRPC_SRC})
    list(APPEND GENERATED_HEADERS ${PROTO_HDR} ${GRPC_HDR})
endforeach()

# ============================================================================
# Library Target
# ============================================================================

# Create static library with generated protos
# Note: Using STATIC because protobuf-generated symbols don't export
# properly on Windows DLLs without explicit export macros
add_library(cryo-protocol STATIC
    ${GENERATED_SOURCES}
    ${GENERATED_HEADERS}
)

# Set C++ standard
target_compile_features(cryo-protocol PUBLIC cxx_std_17)

# Include directories
target_include_directories(cryo-protocol
    PUBLIC
        $<BUILD_INTERFACE:${CRYO_GENERATED_DIR}>  # For proto headers
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${Protobuf_INCLUDE_DIRS}
)

# Link against protobuf and gRPC
target_link_libraries(cryo-protocol
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc++
)

# Set output directory for static library
set_target_properties(cryo-protocol PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # .lib static library
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # Linux .a
)

# ============================================================================
# Custom Target for Proto Generation (optional - for manual regeneration)
# ============================================================================

add_custom_target(cryo-protocol-generate
    DEPENDS ${GENERATED_SOURCES} ${GENERATED_HEADERS}
    COMMENT "Generating all Cryo Protocol C++ files"
)

# ============================================================================
# Installation Rules (optional)
# ============================================================================

install(TARGETS cryo-protocol
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES ${GENERATED_HEADERS} DESTINATION include/cryo-protocol/proto)
install(DIRECTORY ${CRYO_PROTO_DIR}/ DESTINATION share/cryo-protocol/proto
    FILES_MATCHING PATTERN "*.proto"
)

# ============================================================================
# Status Output
# ============================================================================

message(STATUS "Cryo Protocol configured:")
message(STATUS "  Proto dir: ${CRYO_PROTO_DIR}")
message(STATUS "  Generated: ${CRYO_GENERATED_DIR}")
message(STATUS "  Proto files found: ${CRYO_PROTO_FILES}")
message(STATUS "  Total proto files: ${CMAKE_MATCH_COUNT}")

# Count proto files
list(LENGTH CRYO_PROTO_FILES PROTO_COUNT)
message(STATUS "  Proto file count: ${PROTO_COUNT}")