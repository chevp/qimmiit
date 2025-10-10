# coregfx Examples CMake Configuration
cmake_minimum_required(VERSION 3.15)

project(coregfx_examples)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find coregfx library (if installed, otherwise use parent directory)
if(NOT TARGET coregfx)
    # If building as part of parent project, coregfx target should exist
    # Otherwise, find it as a package
    find_package(coregfx CONFIG)
endif()

# Example executable
add_executable(coregfx_example
    main.cpp
)

# Link against coregfx
target_link_libraries(coregfx_example PRIVATE coregfx)

# Set output directory
set_target_properties(coregfx_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(coregfx_example PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

# Installation rules (optional)
install(TARGETS coregfx_example
    RUNTIME DESTINATION bin
)
