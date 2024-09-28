cmake_minimum_required(VERSION 3.28)
project(imagetool)

set(CMAKE_CXX_STANDARD 23)

add_executable(imagetool main.cpp
        concepts.hpp
        util.hpp)

target_link_libraries(imagetool PRIVATE crypto++)