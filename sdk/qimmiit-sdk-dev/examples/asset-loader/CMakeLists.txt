cmake_minimum_required(VERSION 3.20)
project(AssetLoaderExample)

add_executable(asset-loader-example
    main.cpp
)

target_link_libraries(asset-loader-example PRIVATE
    Qimmiit::SDK
)

set_target_properties(asset-loader-example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)