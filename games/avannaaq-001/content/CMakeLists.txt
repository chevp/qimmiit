cmake_minimum_required(VERSION 3.20)

# =============================================================================
# Game Content Library
# Compiles .proto schemas and .pbtxt data files
# =============================================================================

find_package(Protobuf REQUIRED)

# Proto files location
set(CONTENT_PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(CONTENT_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/cpp")

file(MAKE_DIRECTORY ${CONTENT_GENERATED_DIR})

# Find all proto files
file(GLOB CONTENT_PROTO_FILES "${CONTENT_PROTO_DIR}/*.proto")

# Get protoc executable
get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
if(NOT PROTOBUF_PROTOC_EXECUTABLE)
    get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_DEBUG)
endif()

# Generate C++ from proto files
set(CONTENT_PROTO_SOURCES "")
set(CONTENT_PROTO_HEADERS "")

foreach(PROTO_FILE ${CONTENT_PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    set(PROTO_SRC "${CONTENT_GENERATED_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${CONTENT_GENERATED_DIR}/${PROTO_NAME}.pb.h")

    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --proto_path=${CONTENT_PROTO_DIR}
             --cpp_out=${CONTENT_GENERATED_DIR}
             ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating C++ from ${PROTO_NAME}.proto"
        VERBATIM
    )

    list(APPEND CONTENT_PROTO_SOURCES ${PROTO_SRC})
    list(APPEND CONTENT_PROTO_HEADERS ${PROTO_HDR})
endforeach()

message(STATUS "Avannaaq Content configured:")
message(STATUS "  Proto dir: ${CONTENT_PROTO_DIR}")
message(STATUS "  Generated: ${CONTENT_GENERATED_DIR}")
message(STATUS "  Proto files: ${CONTENT_PROTO_FILES}")

# =============================================================================
# Content Library Target
# =============================================================================
add_library(avannaaq-content STATIC
    ${CONTENT_PROTO_SOURCES}
)

target_include_directories(avannaaq-content
    PUBLIC
        ${CONTENT_GENERATED_DIR}
)

target_link_libraries(avannaaq-content
    PUBLIC
        protobuf::libprotobuf
)

# =============================================================================
# Compile .pbtxt files to binary .pb (Custom target)
# =============================================================================
add_custom_target(compile-content
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling .pbtxt files to binary..."
    COMMENT "Compiling game content data files"
)

# TODO: Add custom commands to compile each .pbtxt file
# Example:
# add_custom_command(TARGET compile-content POST_BUILD
#     COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
#         --encode=avannaaq.World
#         ${CONTENT_PROTO_DIR}/world.proto
#         < ${CMAKE_CURRENT_SOURCE_DIR}/worlds/overworld.pbtxt
#         > ${CMAKE_BINARY_DIR}/data/worlds/overworld.pb
# )
