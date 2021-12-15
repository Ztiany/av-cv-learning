cmake_minimum_required(VERSION 3.10.2)
project("android_rtmp")

#添加 LibRTC 子模块
add_subdirectory(librtmp)

#构建共享库
add_library(
    ###库名
    android_rtmp

    ###共享库
    SHARED

    ###源码
    native-lib.cpp
    codec/X264Codec.cpp
    pusher/RtmpPusher.h
    utils/JavaCaller.h
    utils/SafeQueue.h
    utils/SafeQueueCPP11.h
)

#找到依赖库
find_library(log-lib log)

#链接
target_link_libraries(android_rtmp ${log-lib} rtmp)
