cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-ggdb3 -std=c++14")

aux_source_directory(lib sources)

include_directories(include)

add_executable(ss ${sources})
