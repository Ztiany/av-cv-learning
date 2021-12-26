cmake_minimum_required(VERSION 3.4.1)

include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")

message(${CMAKE_SOURCE_DIR})

add_library(
        native-lib
        SHARED
        native-lib.cpp
)

target_link_libraries(
        native-lib
        opencv_java4
        log
        android
)