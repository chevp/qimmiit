cmake_minimum_required(VERSION 3.20)
project(AvannaaqClient)

# =============================================================================
# Game Client Executable
# =============================================================================
add_executable(avannaaq-client
    src/main.cpp
)

target_include_directories(avannaaq-client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link with Qimmiit SDK (for rendering and asset management)
target_link_libraries(avannaaq-client
    PRIVATE
        Qimmiit::SDK
        protobuf::libprotobuf
)

# Link with game content
if(TARGET avannaaq-content)
    target_link_libraries(avannaaq-client PRIVATE avannaaq-content)
endif()

set_target_properties(avannaaq-client PROPERTIES
    OUTPUT_NAME "avannaaq-client"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "Avannaaq Client configured")
