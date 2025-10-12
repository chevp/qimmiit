cmake_minimum_required(VERSION 3.15)
project(umiak-agent VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Generate protobuf and gRPC code
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/umiak.proto
)

# Output directory for generated files
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

# Generate C++ code from proto files
set(PROTO_SRCS)
set(PROTO_HDRS)
set(GRPC_SRCS)
set(GRPC_HDRS)

foreach(proto_file ${PROTO_FILES})
    get_filename_component(proto_name ${proto_file} NAME_WE)

    list(APPEND PROTO_SRCS "${PROTO_OUT_DIR}/${proto_name}.pb.cc")
    list(APPEND PROTO_HDRS "${PROTO_OUT_DIR}/${proto_name}.pb.h")
    list(APPEND GRPC_SRCS "${PROTO_OUT_DIR}/${proto_name}.grpc.pb.cc")
    list(APPEND GRPC_HDRS "${PROTO_OUT_DIR}/${proto_name}.grpc.pb.h")

    add_custom_command(
        OUTPUT "${PROTO_OUT_DIR}/${proto_name}.pb.cc"
               "${PROTO_OUT_DIR}/${proto_name}.pb.h"
               "${PROTO_OUT_DIR}/${proto_name}.grpc.pb.cc"
               "${PROTO_OUT_DIR}/${proto_name}.grpc.pb.h"
        COMMAND protobuf::protoc
        ARGS --grpc_out=${PROTO_OUT_DIR}
             --cpp_out=${PROTO_OUT_DIR}
             --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
             -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
             ${proto_file}
        DEPENDS ${proto_file}
        COMMENT "Generating protobuf and gRPC code for ${proto_name}"
    )
endforeach()

# Umiak agent executable
add_executable(umiak-agent
    src/main.cpp
    src/UmiakService.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

target_include_directories(umiak-agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROTO_OUT_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(umiak-agent PRIVATE
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

# Install target
install(TARGETS umiak-agent
    RUNTIME DESTINATION bin
)

# Optional: Install proto files for reference
install(FILES ${PROTO_FILES}
    DESTINATION share/umiak-agent/proto
)
