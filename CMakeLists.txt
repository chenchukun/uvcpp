cmake_minimum_required(VERSION 2.8)
project(libuvcpp)

set(CMAKE_BUILD_TYPE Debug)

set(VERSION "0.1.0")

set(CMAKE_VERBOSE_MAKEFILE ON)

option(BUILD_SHARED_LIBS "build uvcpp shared library" OFF)
option(BUILD_STATIC_LIBS "build uvcpp static library" ON)

set(CMAKE_C_FLAGS  "-g")
set(CMAKE_CXX_FLAGS "-std=c++11")

add_subdirectory(src)
