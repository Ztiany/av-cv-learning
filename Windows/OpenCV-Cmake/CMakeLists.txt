cmake_minimum_required(VERSION 3.12)
project(opencv_windows)

# 指定 C/C++ 标准
set(CMAKE_CXX_STANDARD 14)

# 解决 CLion + MSVC 下的字符编码问题：https://www.cnblogs.com/Chary/p/13608011.html
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# Clion 配置的工具链是 msys2/mingw64，而通过 mingw64 下载的 opencv 自带了 cmake 的配置文件，因此可以使用下命令进行配置。
include_directories(/mingw64/include/opencv4)
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV_LIBS '${OpenCV_LIBS}'")
message(STATUS "OpenCV found in '${OpenCV_DIR}'")

# aux_source_directory() 是 CMake 中的一个函数，用于将指定目录中的源文件自动添加到 CMake 构建脚本中。
aux_source_directory(base dir_base)

add_executable(opencv_windows main.cpp ${dir_base})
target_link_libraries(opencv_windows ${OpenCV_LIBS})
