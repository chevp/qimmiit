cmake_minimum_required(VERSION 3.20)
project(StudioIntegrationExample)

add_executable(studio-integration-example
    main.cpp
)

target_link_libraries(studio-integration-example PRIVATE
    Qimmiit::SDK
)

set_target_properties(studio-integration-example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)
