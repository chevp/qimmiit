cmake_minimum_required(VERSION 3.20)
project(AvannaaqServer)

# =============================================================================
# Game Server Executable
# =============================================================================
add_executable(avannaaq-server
    src/main.cpp
    src/game_loop.cpp
    src/game_loop.hpp

    # Network layer
    src/network/connection_manager.cpp
    src/network/connection_manager.hpp
    src/network/packet_handler.cpp
    src/network/packet_handler.hpp

    # Gameplay systems
    src/gameplay/world_manager.cpp
    src/gameplay/world_manager.hpp
    src/gameplay/player_manager.cpp
    src/gameplay/player_manager.hpp
    src/gameplay/combat_system.cpp
    src/gameplay/combat_system.hpp

    # Storage layer
    src/storage/game_state_store.cpp
    src/storage/game_state_store.hpp
)

target_include_directories(avannaaq-server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../content/proto
)

# Link with Qimmiit SDK
target_link_libraries(avannaaq-server
    PRIVATE
        Qimmiit::SDK
        protobuf::libprotobuf
        nlohmann_json::nlohmann_json
        SQLite::SQLite3
)

# Link with game content library
if(TARGET avannaaq-content)
    target_link_libraries(avannaaq-server PRIVATE avannaaq-content)
endif()

set_target_properties(avannaaq-server PROPERTIES
    OUTPUT_NAME "avannaaq-server"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(avannaaq-server PRIVATE
        _WIN32_WINNT=0x0601
        WIN32_LEAN_AND_MEAN
    )
    target_link_libraries(avannaaq-server PRIVATE ws2_32)
endif()

message(STATUS "Avannaaq Server configured")
