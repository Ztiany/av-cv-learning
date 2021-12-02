cmake_minimum_required(VERSION 3.10.2)
project("android_rtmp")

#添加 LibRTC 子模块
add_subdirectory(librtmp)

#添加源码
aux_source_directory(./jni JNI_MODULE)

#构建共享库
add_library(android_rtmp SHARED ${JNI_MODULE})

#找到依赖库
find_library(log-lib log)

#链接
target_link_libraries(android_rtmp ${log-lib} rtmp)
