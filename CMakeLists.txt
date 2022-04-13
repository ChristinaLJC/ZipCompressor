cmake_minimum_required(VERSION 3.21)
project(ZipCompression2.0)

set(CMAKE_CXX_STANDARD 17)

add_executable(ZipCompression compression.cpp lz77.cpp)

add_executable(generate generate.cpp)