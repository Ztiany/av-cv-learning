cmake_minimum_required(VERSION 3.10.2)
project("androidav")


add_library(androidav SHARED native-lib.cpp common/times.c)

find_library(log-lib log)

target_link_libraries(androidav ${log-lib})