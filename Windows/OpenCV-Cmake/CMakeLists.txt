cmake_minimum_required(VERSION 3.12)
project(opencv_windows)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# indicate the location of header files
include_directories(D:/dev_tools/msys64/mingw64/include/opencv4)

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV_LIBS '${OpenCV_LIBS}'")
message(STATUS "OpenCV found in '${OpenCV_DIR}'")

aux_source_directory(base dir_base)

add_executable(opencv_windows main.cpp ${dir_base})
target_link_libraries(opencv_windows ${OpenCV_LIBS})