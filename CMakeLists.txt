cmake_minimum_required(VERSION 3.14)
project(mystd)
set(CMAKE_CXX_STANDARD 20)

add_library(mystd INTERFACE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_include_directories(mystd INTERFACE include/)
add_subdirectory(tests)

