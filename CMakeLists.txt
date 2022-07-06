cmake_minimum_required(VERSION 3.23)
project(x1000)
set(CMAKE_CXX_STANDARD 23)

# Basic CMake setup
include(FindPkgConfig)

# Setup conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Find SDL2
pkg_search_module(SDL2 REQUIRED sdl2)

add_executable(${CMAKE_PROJECT_NAME}
    source/globals.cpp
    source/main.cpp
    source/render.cpp
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)

target_link_libraries(${CMAKE_PROJECT_NAME}
    fmt
    ${SDL2_LIBRARIES}
    SDL2_ttf
)
