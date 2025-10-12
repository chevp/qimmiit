cmake_minimum_required(VERSION 3.15)
project(umiak-agent VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Umiak agent executable - pure C++, no protobuf needed
add_executable(umiak-agent
    src/main.cpp
    src/UmiakAgent.cpp
)

target_include_directories(umiak-agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Install target
install(TARGETS umiak-agent
    RUNTIME DESTINATION bin
)
