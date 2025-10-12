# =============================================================================
# Qimmiit SDK Release Generator
# =============================================================================
#
# This script packages the Qimmiit SDK for distribution
#
# Usage:
#   cmake -P generate-release-sdk.cmake
#
# Or with custom options:
#   cmake -DQIMMIIT_VERSION=1.0.0 -DOUTPUT_DIR=dist/sdk -P generate-release-sdk.cmake
#

cmake_minimum_required(VERSION 3.20)

# =============================================================================
# Configuration
# =============================================================================

if(NOT DEFINED QIMMIIT_VERSION)
    set(QIMMIIT_VERSION "1.0.0")
endif()

if(NOT DEFINED OUTPUT_DIR)
    set(OUTPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../dist/sdk")
endif()

if(NOT DEFINED PLATFORM)
    if(WIN32)
        set(PLATFORM "win64")
    elseif(UNIX AND NOT APPLE)
        set(PLATFORM "linux")
    elseif(APPLE)
        set(PLATFORM "macos")
    else()
        set(PLATFORM "unknown")
    endif()
endif()

set(SDK_NAME "qimmiit-sdk-${QIMMIIT_VERSION}-${PLATFORM}")
set(SDK_OUTPUT_PATH "${OUTPUT_DIR}/${SDK_NAME}")

message(STATUS "=============================================================================")
message(STATUS "Qimmiit SDK Release Generator")
message(STATUS "=============================================================================")
message(STATUS "Version: ${QIMMIIT_VERSION}")
message(STATUS "Platform: ${PLATFORM}")
message(STATUS "Output: ${SDK_OUTPUT_PATH}")
message(STATUS "")

# =============================================================================
# Build SDK
# =============================================================================

message(STATUS "Building Qimmiit SDK...")

set(SDK_BUILD_DIR "${CMAKE_CURRENT_LIST_DIR}/../qimmiit-sdk-dev/build-release")

# Configure
execute_process(
    COMMAND ${CMAKE_COMMAND}
        -S "${CMAKE_CURRENT_LIST_DIR}/../qimmiit-sdk-dev"
        -B "${SDK_BUILD_DIR}"
        -DCMAKE_BUILD_TYPE=Release
        -DQIMMIIT_SDK_BUILD_EXAMPLES=OFF
        -DQIMMIIT_SDK_INSTALL=ON
        -DCMAKE_INSTALL_PREFIX="${SDK_OUTPUT_PATH}"
    RESULT_VARIABLE CONFIGURE_RESULT
)

if(NOT CONFIGURE_RESULT EQUAL 0)
    message(FATAL_ERROR "SDK configuration failed")
endif()

# Build
execute_process(
    COMMAND ${CMAKE_COMMAND}
        --build "${SDK_BUILD_DIR}"
        --config Release
    RESULT_VARIABLE BUILD_RESULT
)

if(NOT BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "SDK build failed")
endif()

# Install
execute_process(
    COMMAND ${CMAKE_COMMAND}
        --install "${SDK_BUILD_DIR}"
        --config Release
    RESULT_VARIABLE INSTALL_RESULT
)

if(NOT INSTALL_RESULT EQUAL 0)
    message(FATAL_ERROR "SDK installation failed")
endif()

# =============================================================================
# Copy Documentation
# =============================================================================

message(STATUS "Copying documentation...")

file(MAKE_DIRECTORY "${SDK_OUTPUT_PATH}/docs")

# Copy README if exists
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../qimmiit-sdk-dev/README.md")
    file(COPY "${CMAKE_CURRENT_LIST_DIR}/../qimmiit-sdk-dev/README.md"
         DESTINATION "${SDK_OUTPUT_PATH}")
endif()

# =============================================================================
# Create SDK Info File
# =============================================================================

message(STATUS "Creating SDK info file...")

file(WRITE "${SDK_OUTPUT_PATH}/SDK_INFO.txt"
"Qimmiit SDK ${QIMMIIT_VERSION}
Platform: ${PLATFORM}
Generated: ${CMAKE_CURRENT_TIMESTAMP}

Directory Structure:
  include/    - Public headers
  lib/        - Libraries
  cmake/      - CMake configuration files
  docs/       - Documentation

Usage:
  In your CMakeLists.txt:
    find_package(QimmiitSDKDev REQUIRED)
    target_link_libraries(your-target PRIVATE Qimmiit::SDK)

For more information, see README.md
")

# =============================================================================
# Create Archive
# =============================================================================

message(STATUS "Creating archive...")

set(ARCHIVE_NAME "${SDK_NAME}.zip")
set(ARCHIVE_PATH "${OUTPUT_DIR}/${ARCHIVE_NAME}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cf "${ARCHIVE_PATH}" --format=zip "${SDK_NAME}"
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    RESULT_VARIABLE ARCHIVE_RESULT
)

if(NOT ARCHIVE_RESULT EQUAL 0)
    message(WARNING "Archive creation failed, but SDK files are available in ${SDK_OUTPUT_PATH}")
else()
    message(STATUS "Archive created: ${ARCHIVE_PATH}")
endif()

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "=============================================================================")
message(STATUS "SDK Generation Complete!")
message(STATUS "=============================================================================")
message(STATUS "SDK Location: ${SDK_OUTPUT_PATH}")
if(ARCHIVE_RESULT EQUAL 0)
    message(STATUS "Archive: ${ARCHIVE_PATH}")
endif()
message(STATUS "")
message(STATUS "To use this SDK in your project:")
message(STATUS "  1. Extract/copy SDK to your desired location")
message(STATUS "  2. In CMakeLists.txt add:")
message(STATUS "       set(CMAKE_PREFIX_PATH /path/to/${SDK_NAME})")
message(STATUS "       find_package(QimmiitSDKDev REQUIRED)")
message(STATUS "       target_link_libraries(your-target PRIVATE Qimmiit::SDK)")
message(STATUS "")
