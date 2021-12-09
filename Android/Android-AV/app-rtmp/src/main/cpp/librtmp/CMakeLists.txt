# 关闭 ssl，LibRTMP 默认支持 SSL，这里没有则需要关闭。
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")

# 收集源文件
#file(GLOB rtmp_source *.c)

# 编译成 rtmp 静态库
add_library(
        ###库名
        rtmp

        ###静态库
        STATIC

        ###源码
        amf.c
        hashswf.c
        log.c
        parseurl.c
        rtmp.c
)