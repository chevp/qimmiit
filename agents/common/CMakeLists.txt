# Agent Shared Library
#
# Provides common interfaces and base classes for all Qimmiit agents.
# This library has NO dependencies on the Qimmiit engine - it's completely independent.

cmake_minimum_required(VERSION 3.20)
project(agent-shared)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Generate protobuf/gRPC code
set(PROTO_FILES
    proto/agent_common.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Get the directory where protobuf generates code
get_filename_component(PROTO_INCLUDE_DIR ${PROTO_HDRS} DIRECTORY)

# Generate gRPC code
set(GRPC_SRCS)
set(GRPC_HDRS)
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)

    set(GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")

    add_custom_command(
        OUTPUT ${GRPC_SRC} ${GRPC_HDR}
        COMMAND protobuf::protoc
        ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
             --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
             -I${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_DIR}
             ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating gRPC code for ${PROTO_FILE}"
    )

    list(APPEND GRPC_SRCS ${GRPC_SRC})
    list(APPEND GRPC_HDRS ${GRPC_HDR})
endforeach()

# Create header-only library for the interface
add_library(agent-shared INTERFACE)

target_include_directories(agent-shared INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Create static library with protobuf implementations
add_library(agent-shared-proto STATIC
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

target_include_directories(agent-shared-proto PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}  # For generated protobuf headers
)

target_link_libraries(agent-shared-proto PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
)

# Installation
install(TARGETS agent-shared-proto
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/agent-shared
)

install(FILES ${PROTO_HDRS} ${GRPC_HDRS}
    DESTINATION include/agent-shared/proto
)

# Export targets
export(TARGETS agent-shared agent-shared-proto
    FILE ${CMAKE_CURRENT_BINARY_DIR}/AgentSharedTargets.cmake
)
