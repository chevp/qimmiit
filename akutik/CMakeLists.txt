cmake_minimum_required(VERSION 3.20)
project(akutik VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(AKUTIK_SOURCES
    src/Storage.cpp
    src/tools/Ulu.cpp
    src/tools/Niqi.cpp
    src/tools/IgluStore.cpp
)

# Header files
set(AKUTIK_HEADERS
    include/akutik/Akutik.hpp
    include/akutik/Storage.hpp
    include/akutik/StorageBackend.hpp
    include/akutik/tools/Ulu.hpp
    include/akutik/tools/Niqi.hpp
    include/akutik/tools/IgluStore.hpp
)

# Create library
add_library(akutik STATIC
    ${AKUTIK_SOURCES}
    ${AKUTIK_HEADERS}
)

# Include directories
target_include_directories(akutik
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Find dependencies
find_package(SQLite3 REQUIRED)

# Link dependencies
target_link_libraries(akutik
    PUBLIC
        SQLite::SQLite3
)

# Compiler options
if(MSVC)
    target_compile_options(akutik PRIVATE /W4)
else()
    target_compile_options(akutik PRIVATE -Wall -Wextra -pedantic)
endif()

# Set properties
set_target_properties(akutik PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${AKUTIK_HEADERS}"
)

# Installation rules
install(TARGETS akutik
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/akutik
)