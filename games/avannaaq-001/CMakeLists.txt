cmake_minimum_required(VERSION 3.20)
project(Avannaaq001 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "=============================================================================")
message(STATUS "Avannaaq-001 Game Ecosystem")
message(STATUS "=============================================================================")

# =============================================================================
# Options
# =============================================================================
option(AVANNAAQ_BUILD_SERVER "Build game server" ON)
option(AVANNAAQ_BUILD_CLIENT "Build game client" ON)
option(AVANNAAQ_BUILD_CMS "Build content management system" ON)
option(AVANNAAQ_BUILD_CONTENT "Compile game content" ON)

# =============================================================================
# Find Qimmiit SDK
# =============================================================================
# Try to find SDK in parent repository
set(QIMMIIT_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/qimmiit-sdk-dev")

if(EXISTS "${QIMMIIT_SDK_PATH}/CMakeLists.txt")
    message(STATUS "Found Qimmiit SDK at: ${QIMMIIT_SDK_PATH}")
    add_subdirectory(${QIMMIIT_SDK_PATH} ${CMAKE_BINARY_DIR}/qimmiit-sdk)
else()
    message(FATAL_ERROR "Qimmiit SDK not found at ${QIMMIIT_SDK_PATH}")
endif()

# =============================================================================
# Find Dependencies
# =============================================================================
find_package(Protobuf CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)

message(STATUS "Dependencies:")
message(STATUS "  Protobuf: ${Protobuf_VERSION}")
message(STATUS "  JSON: ${nlohmann_json_VERSION}")
message(STATUS "  SQLite3: ${SQLite3_VERSION}")

# =============================================================================
# Global Settings
# =============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =============================================================================
# Build Game Content (Proto files)
# =============================================================================
if(AVANNAAQ_BUILD_CONTENT)
    message(STATUS "Adding game content...")
    add_subdirectory(content)
endif()

# =============================================================================
# Build Runtime Server
# =============================================================================
if(AVANNAAQ_BUILD_SERVER)
    message(STATUS "Adding game server...")
    add_subdirectory(runtime/server)
endif()

# =============================================================================
# Build Runtime Client
# =============================================================================
if(AVANNAAQ_BUILD_CLIENT)
    message(STATUS "Adding game client...")
    add_subdirectory(runtime/client)
endif()

# =============================================================================
# Build Content Management System
# =============================================================================
if(AVANNAAQ_BUILD_CMS)
    message(STATUS "Adding content management system...")
    add_subdirectory(content-management)
endif()

# =============================================================================
# Summary
# =============================================================================
message(STATUS "")
message(STATUS "Avannaaq-001 Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Server: ${AVANNAAQ_BUILD_SERVER}")
message(STATUS "  Build Client: ${AVANNAAQ_BUILD_CLIENT}")
message(STATUS "  Build CMS: ${AVANNAAQ_BUILD_CMS}")
message(STATUS "  Build Content: ${AVANNAAQ_BUILD_CONTENT}")
message(STATUS "")
message(STATUS "=============================================================================")
