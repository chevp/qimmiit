# Orchestrators CMakeLists.txt
# Orchestration systems for the Qimmiit Engine

cmake_minimum_required(VERSION 3.20)

message(STATUS "Adding orchestrators...")

# Add Nanook orchestrator
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/nanook/CMakeLists.txt")
    message(STATUS "  - Adding Nanook orchestrator")
    add_subdirectory(nanook)
else()
    message(WARNING "  - Nanook orchestrator not found")
endif()
