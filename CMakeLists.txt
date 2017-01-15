cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-ggdb3 -std=c++14")

aux_source_directory(lib sources)
aux_source_directory(test test_sources)

find_package(GTest REQUIRED)

include_directories(include)

add_executable(ss app/ss.cpp ${sources})

add_executable(ss_test ${sources} ${test_sources})

target_link_libraries(ss_test GTest::GTest GTest::Main)
