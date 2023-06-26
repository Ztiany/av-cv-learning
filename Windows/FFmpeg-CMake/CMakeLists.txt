cmake_minimum_required(VERSION 3.15)
project(FFmpegWindows_Clion)

set(CMAKE_CXX_STANDARD 14)

#能够在编码的时候使用ffmpeg的函数
include_directories(D:/dev_tools/msys64/usr/local/ffmpeg/include)
# 设置引导链接库的查找目录
link_directories(D:/dev_tools/msys64/usr/local/ffmpeg/lib)

#可执行文件
add_executable(FFmpegWindows_Clion ffmpeg-main.cpp)

#设置编译链接的库 ffmpeg（动态库 avutil avcodec avfilter avdevice avformat postproc swresample 的路径必须添加到环境变量中）
target_link_libraries(FFmpegWindows_Clion avutil avcodec avfilter avdevice avformat postproc swresample)