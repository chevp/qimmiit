cmake_minimum_required(VERSION 3.20)
project(QimmiitSDKDev VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Configuring Qimmiit SDK Dev...")

# =============================================================================
# Options
# =============================================================================
option(QIMMIIT_SDK_BUILD_EXAMPLES "Build SDK examples" ON)
option(QIMMIIT_SDK_INSTALL "Install SDK components" OFF)

# =============================================================================
# Find Dependencies
# =============================================================================
find_package(Vulkan REQUIRED)

# =============================================================================
# Import Qimmiit Modules
# =============================================================================

# Rendering Module
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../modules/rendering/coregfx/CMakeLists.txt")
    message(STATUS "Adding coregfx module...")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../modules/rendering/coregfx
                     ${CMAKE_BINARY_DIR}/modules/coregfx)
    set(QIMMIIT_HAS_COREGFX TRUE)
else()
    message(WARNING "coregfx module not found")
    set(QIMMIIT_HAS_COREGFX FALSE)
endif()

# Asset Modules
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../modules/assets/cryo-asset/CMakeLists.txt")
    message(STATUS "Adding cryo-asset module...")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../modules/assets/cryo-asset
                     ${CMAKE_BINARY_DIR}/modules/cryo-asset)
    set(QIMMIIT_HAS_CRYO_ASSET TRUE)
else()
    message(WARNING "cryo-asset module not found")
    set(QIMMIIT_HAS_CRYO_ASSET FALSE)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../modules/assets/cryo-cache/CMakeLists.txt")
    message(STATUS "Adding cryo-cache module...")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../modules/assets/cryo-cache
                     ${CMAKE_BINARY_DIR}/modules/cryo-cache)
    set(QIMMIIT_HAS_CRYO_CACHE TRUE)
else()
    message(WARNING "cryo-cache module not found")
    set(QIMMIIT_HAS_CRYO_CACHE FALSE)
endif()

# Tooling Modules
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../modules/toolings/akutik/CMakeLists.txt")
    message(STATUS "Adding akutik module...")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../modules/toolings/akutik
                     ${CMAKE_BINARY_DIR}/modules/akutik)
    set(QIMMIIT_HAS_AKUTIK TRUE)
else()
    message(WARNING "akutik module not found")
    set(QIMMIIT_HAS_AKUTIK FALSE)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../modules/toolings/cryo-tooling/CMakeLists.txt")
    message(STATUS "Adding cryo-tooling module...")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../modules/toolings/cryo-tooling
                     ${CMAKE_BINARY_DIR}/modules/cryo-tooling)
    set(QIMMIIT_HAS_CRYO_TOOLING TRUE)
else()
    message(WARNING "cryo-tooling module not found")
    set(QIMMIIT_HAS_CRYO_TOOLING FALSE)
endif()

# =============================================================================
# Create SDK Interface Library
# =============================================================================
add_library(qimmiit-sdk-dev INTERFACE)
add_library(Qimmiit::SDK ALIAS qimmiit-sdk-dev)

# Link available modules
if(QIMMIIT_HAS_COREGFX AND TARGET coregfx)
    target_link_libraries(qimmiit-sdk-dev INTERFACE coregfx)
endif()

if(QIMMIIT_HAS_CRYO_ASSET AND TARGET cryo-asset)
    target_link_libraries(qimmiit-sdk-dev INTERFACE cryo-asset)
endif()

if(QIMMIIT_HAS_CRYO_CACHE AND TARGET cryo-cache)
    target_link_libraries(qimmiit-sdk-dev INTERFACE cryo-cache)
endif()

if(QIMMIIT_HAS_AKUTIK AND TARGET akutik)
    target_link_libraries(qimmiit-sdk-dev INTERFACE akutik)
endif()

if(QIMMIIT_HAS_CRYO_TOOLING AND TARGET cryo-tooling)
    target_link_libraries(qimmiit-sdk-dev INTERFACE cryo-tooling)
endif()

# Add SDK include directory for unified headers
target_include_directories(qimmiit-sdk-dev INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# =============================================================================
# SDK Configuration Summary
# =============================================================================
message(STATUS "")
message(STATUS "Qimmiit SDK Dev Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Modules:")
message(STATUS "    - coregfx:      ${QIMMIIT_HAS_COREGFX}")
message(STATUS "    - cryo-asset:   ${QIMMIIT_HAS_CRYO_ASSET}")
message(STATUS "    - cryo-cache:   ${QIMMIIT_HAS_CRYO_CACHE}")
message(STATUS "    - akutik:       ${QIMMIIT_HAS_AKUTIK}")
message(STATUS "    - cryo-tooling: ${QIMMIIT_HAS_CRYO_TOOLING}")
message(STATUS "")

# =============================================================================
# Examples
# =============================================================================
if(QIMMIIT_SDK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# =============================================================================
# Installation
# =============================================================================
if(QIMMIIT_SDK_INSTALL)
    include(GNUInstallDirs)

    install(TARGETS qimmiit-sdk-dev
        EXPORT QimmiitSDKDevTargets
    )

    install(EXPORT QimmiitSDKDevTargets
        FILE QimmiitSDKDevTargets.cmake
        NAMESPACE Qimmiit::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QimmiitSDKDev
    )

    # Install SDK headers if they exist
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
        install(DIRECTORY include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()

    # Generate and install package config file
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QimmiitSDKDevConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/QimmiitSDKDevConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QimmiitSDKDev
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/QimmiitSDKDevConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/QimmiitSDKDevConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/QimmiitSDKDevConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QimmiitSDKDev
    )
endif()