# 设置 CMake 版本
cmake_minimum_required(VERSION 3.15)
# 设置项目名称
project(FFmpeg_CMake)
# 设置 C++ 版本
set(CMAKE_CXX_STANDARD 14)

# 设置头文件路径
include_directories(D:/dev_tools/msys64/usr/local/ffmpeg/include)
# 设置 .lib 导入库的查找路径
link_directories(D:/dev_tools/msys64/usr/local/ffmpeg/lib)

# 编译为执行文件
add_executable(FFmpeg_CMake ffmpeg-main.cpp)

# 设置编译的时候，链接 FFmpeg 的导入库
# 另外，选用将动态库：avutil avcodec avfilter avdevice avformat postproc swresample 路径添加到环境变量中，因为运行时需要用到。
target_link_libraries(FFmpeg_CMake avutil avcodec avfilter avdevice avformat postproc swresample)