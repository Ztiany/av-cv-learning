#ifndef OPENGL_BASIC_GLKIT_H
#define OPENGL_BASIC_GLKIT_H

#include <Windows.h>
#include <gl/GL.h>

HGLRC makeGLRenderEnv(HWND hwnd);

typedef void (*WindowProgramCallback)(HWND hwnd, HGLRC hGLRC);

void startGlWindowProgram(
        HINSTANCE hInstance,
        WNDPROC glWindowProc,
        int windowWidth,
        int windowHeight,
        WindowProgramCallback callback
);

#endif //OPENGL_BASIC_GLKIT_H
