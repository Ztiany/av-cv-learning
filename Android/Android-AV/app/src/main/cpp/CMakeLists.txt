cmake_minimum_required(VERSION 3.10.2)
project("androidav")

#添加头文件
include_directories(include)
include_directories(include/Glm/core)
include_directories(include/Glm/gtx)
include_directories(include/Glm/gtc)
include_directories(include/Glm/virtrev)
include_directories(include/Glm)
#查找指定目录下的所有源文件，并将名称保存到 DIR_SRCS 变量
aux_source_directory(module DIR_MODULE)
aux_source_directory(module/common DIR_COMMON)
aux_source_directory(module/opengl DIR_OPENGL)

#构建共享库
add_library(
        androidav
        SHARED
        ${DIR_MODULE}
        ${DIR_COMMON}
        ${DIR_OPENGL}
)

#找到依赖库
find_library(log-lib log)
find_library(GLESv2-lib GlESv2)

#链接
target_link_libraries(androidav ${log-lib} android ${GLESv2-lib})
