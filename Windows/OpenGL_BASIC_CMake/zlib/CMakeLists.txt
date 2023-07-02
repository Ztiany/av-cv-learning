set(
        zlib_files
        WindowKit.cpp
        GLKit.cpp
        LogKit.cpp
        TextureKit.cpp
        IOKit.cpp
)

add_library(zlib ${zlib_files})
target_link_libraries(zlib ${OPENGL_LIBRARY})

install(TARGETS zlib DESTINATION lib)