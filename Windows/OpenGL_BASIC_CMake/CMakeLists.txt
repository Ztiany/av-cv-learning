cmake_minimum_required(VERSION 3.23)
project(OpenGL_BASIC)

# 指定 C/C++ 标准
set(CMAKE_CXX_STANDARD 14)

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# 设置一个日志宏
add_compile_definitions(OPEN_LOG=1)

# OpenGL
find_package(OpenGL REQUIRED)

# 解决 CLion + MSVC 下的字符编码问题：https://www.cnblogs.com/Chary/p/13608011.html
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(zlib)

# 添加子项目
add_subdirectory(zlib)
add_subdirectory(01.hello_windows)
add_subdirectory(02.hello_opengl)
add_subdirectory(03.draw_points)
add_subdirectory(04.draw_lines)
add_subdirectory(05.draw_triangle)
add_subdirectory(06.draw_quads)
add_subdirectory(07.draw_polygon)
add_subdirectory(08.matrix_operation)
add_subdirectory(09.matrix_stack)
add_subdirectory(10.lighting)
add_subdirectory(11.texture)
