cmake_minimum_required(VERSION 3.20)

# =============================================================================
# Content Management System
# Tools for managing, validating, and migrating game content
# =============================================================================

# Content CLI Tool
add_executable(content-cli
    tools/content_cli.cpp
    validation/content_validator.cpp
    validation/content_validator.hpp
)

target_include_directories(content-cli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../content/generated/cpp
)

target_link_libraries(content-cli
    PRIVATE
        avannaaq-content
        protobuf::libprotobuf
        nlohmann_json::nlohmann_json
)

set_target_properties(content-cli PROPERTIES
    OUTPUT_NAME "content-cli"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "Content Management System configured")
