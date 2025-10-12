cmake_minimum_required(VERSION 3.20)
project(BasicRendererExample)

add_executable(basic-renderer-example
    main.cpp
)

target_link_libraries(basic-renderer-example PRIVATE
    Qimmiit::SDK
)

# Copy to examples output directory
set_target_properties(basic-renderer-example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)