cmake_minimum_required(VERSION 3.23)
project(x1000 LANGUAGES CXX VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 23)

# Basic CMake setup
include(FindPkgConfig)

# Setup conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Find dependencies
pkg_search_module(SDL2 REQUIRED sdl2)
find_package(Catch2 2 REQUIRED)
find_package(spdlog REQUIRED)

# Set up some common stuff
set(common_sources
    source/bus.cpp
    source/globals.cpp
    source/nes/apu.cpp
    source/nes/cpu.cpp
    source/nes/mem.cpp
    source/nes/ppu.cpp
    source/render.cpp
    source/util.cpp
)

set(common_definitions
    SPDLOG_FMT_EXTERNAL
)

set(common_libraries
    fmt
    ${SDL2_LIBRARIES}
    SDL2_ttf
    Catch2::Catch2
    spdlog::spdlog
    CONAN_PKG::argparse
)

set(common_include_directories
    ${CMAKE_SOURCE_DIR}
)

# Set up main application
add_executable(${CMAKE_PROJECT_NAME}
    ${common_sources}
    source/main.cpp
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    ${common_definitions}
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${common_include_directories}
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -pedantic
    # -Werror
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${common_libraries}
)

# Set up test application
add_executable(${CMAKE_PROJECT_NAME}_test
    ${common_sources}
    source/test.cpp
)

target_compile_definitions(${CMAKE_PROJECT_NAME}_test PRIVATE
    ${common_definitions}
    TEST
)

target_include_directories(${CMAKE_PROJECT_NAME}_test PRIVATE
    ${common_include_directories}
)

target_compile_options(${CMAKE_PROJECT_NAME}_test PRIVATE
    -Wall
    -Wextra
    -pedantic
    # -Werror
)

target_link_libraries(${CMAKE_PROJECT_NAME}_test
    ${common_libraries}
)

include(CTest)
include(Catch)
catch_discover_tests(${CMAKE_PROJECT_NAME}_test)
