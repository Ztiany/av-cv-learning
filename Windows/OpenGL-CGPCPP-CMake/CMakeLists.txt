cmake_minimum_required(VERSION 3.23)
project(OpenGL_CGPCPP)

# 指定 C/C++ 标准
set(CMAKE_CXX_STANDARD 14)

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# 头文件路径
include_directories(./lib/)
# 静态库/引导库路径
link_directories(./lib)

# OpenGL
find_package(OpenGL REQUIRED)

# 解决 CLion + MSVC 下的字符编码问题：https://www.cnblogs.com/Chary/p/13608011.html
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 添加子项目
add_subdirectory(program2.1)
