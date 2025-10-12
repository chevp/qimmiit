cmake_minimum_required(VERSION 3.20)

message(STATUS "Configuring Qimmiit SDK Examples...")

# =============================================================================
# Example 1: Basic Renderer
# =============================================================================
if(QIMMIIT_HAS_COREGFX)
    add_subdirectory(basic-renderer)
endif()

# =============================================================================
# Example 2: Asset Loading
# =============================================================================
if(QIMMIIT_HAS_CRYO_ASSET)
    add_subdirectory(asset-loader)
endif()

# =============================================================================
# Example 3: Full Studio Integration
# =============================================================================
if(QIMMIIT_HAS_COREGFX AND QIMMIIT_HAS_AKUTIK)
    add_subdirectory(studio-integration)
endif()