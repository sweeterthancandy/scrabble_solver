cmake_minimum_required(VERSION 3.7)

SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_CXX_FLAGS "-ggdb3 -std=c++14")

add_executable(ss ss.cpp ss_rack.cpp ss_renderer.cpp ss_strategy.cpp ss_util.cpp ss_dict.cpp)
