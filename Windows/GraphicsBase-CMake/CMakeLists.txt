# 指定 CMake 版本
cmake_minimum_required(VERSION 3.23)
# 指定项目名称
project(GraphicsBase_CMake)

# 指定 C/C++ 标准
set(CMAKE_CXX_STANDARD 14)

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# 解决 CLion + MSVC 下的字符编码问题：https://www.cnblogs.com/Chary/p/13608011.html
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 添加子项目
add_subdirectory(01-creating-window/)
add_subdirectory(02-draw-point/)

