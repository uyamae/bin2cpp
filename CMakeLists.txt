cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 17)
project(bin2cpp)
add_subdirectory(src)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT bin2cpp)
