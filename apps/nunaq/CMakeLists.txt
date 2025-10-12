cmake_minimum_required(VERSION 3.20)
project(Nunaq VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find packages
find_package(glfw3 CONFIG REQUIRED)

# Nunaq executable - simple launcher
add_executable(nunaq src/main.cpp)

# Link libraries
target_link_libraries(nunaq PRIVATE glfw)

# Link Nanook when available
if(TARGET nanook)
    target_link_libraries(nunaq PRIVATE nanook)
    message(STATUS "  Linking with Nanook orchestrator")
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(nunaq PRIVATE /W4)
else()
    target_compile_options(nunaq PRIVATE -Wall -Wextra)
endif()

# Install
install(TARGETS nunaq RUNTIME DESTINATION bin)

# Print configuration
message(STATUS "Nunaq - Simple Window Launcher")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
