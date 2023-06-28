cmake_minimum_required(VERSION 3.12)
project(opencv_windows)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# indicate the location of header files
include_directories(./lib)
# indicate the location of import library which is named libopencv_world411.dll.a
link_directories(./lib)

aux_source_directory(base dir_base)

add_executable(opencv_windows main.cpp  ${dir_base})
target_link_libraries(opencv_windows libopencv_world411)