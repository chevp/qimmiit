cmake_minimum_required(VERSION 3.20)
project(sakku VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Sakku CLI executable
add_executable(sakku-cli
    src/main.cpp
    src/commands/PublishCommand.cpp
    src/commands/SearchCommand.cpp
    src/commands/AddCommand.cpp
    src/registry/RegistryClient.cpp
    src/manifest/ManifestManager.cpp
    src/utils/FileUtils.cpp
)

target_include_directories(sakku-cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sakku-cli PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# Set output directory
set_target_properties(sakku-cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation
install(TARGETS sakku-cli
    RUNTIME DESTINATION bin
)

message(STATUS "Sakku CLI configured successfully")