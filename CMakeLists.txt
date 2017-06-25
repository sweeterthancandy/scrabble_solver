cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-std=c++14")

aux_source_directory(lib lib_sources)
aux_source_directory(test test_sources)

aux_source_directory(app driver_sources)

add_library(ss_lib SHARED ${lib_sources})

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(GTest REQUIRED)

include_directories(include)


add_executable(driver ${driver_sources})
target_link_libraries(driver ss_lib)

#add_executable(ss_test ${test_sources})
#target_link_libraries(ss_test GTest::GTest GTest::Main)
#target_link_libraries(ss_test ss_lib)
