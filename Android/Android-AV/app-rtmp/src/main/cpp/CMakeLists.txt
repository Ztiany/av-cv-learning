cmake_minimum_required(VERSION 3.10.2)
project("android-rtmp")

#添加 LibRTC 子模块
add_subdirectory(librtmp)
#查找指定目录下的所有源文件，并将名称保存到 DIR_SRCS 变量
aux_source_directory(jni JNI_MODULE)

#构建共享库
add_library(
        android-rtmp
        SHARED
        ${JNI_MODULE}
)

#找到依赖库
find_library(log-lib log)

#链接
target_link_libraries(android-rtmp ${log-lib}  rtmp)
