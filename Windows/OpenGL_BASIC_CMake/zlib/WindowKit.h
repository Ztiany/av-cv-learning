#ifndef OPENGL_BASIC_WINDOWKIT_H
#define OPENGL_BASIC_WINDOWKIT_H

#include <Windows.h>

HWND createWindow(
        HINSTANCE hInstance,
        WNDPROC glWindowProc,
        int windowWidth,
        int windowHeight
);

void startWindowMessageLoop();

#endif //OPENGL_BASIC_WINDOWKIT_H
