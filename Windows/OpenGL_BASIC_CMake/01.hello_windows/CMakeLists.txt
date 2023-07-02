# https://stackoverflow.com/questions/48915216/link-error-when-compiling-win32-application-with-clion-cmake-msvc-2015
add_executable(01.hello_windows WIN32 main.cpp)
target_link_libraries(01.hello_windows zlib)
