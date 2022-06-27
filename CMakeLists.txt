cmake_minimum_required(VERSION 3.23)
project(hello)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
add_executable(${CMAKE_PROJECT_NAME} source/main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} fmt)
