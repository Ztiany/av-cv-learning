#include "GLKit.h"
#include <WindowKit.h>
#include <LogKit.h>

HGLRC makeGLRenderEnv(HWND hwnd) {
    HDC dc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    pfd.nVersion = 1;
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

    int pixelFormat = ChoosePixelFormat(dc, &pfd);
    SetPixelFormat(dc, pixelFormat, &pfd);
    HGLRC rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);

    return rc;
}

/**
 * 开始一个 OpenGL 的窗口程序。
 *
 * @param glWindowProc 窗口过程。
 * @param windowWidth 窗口的宽。
 * @param windowHeight 窗口的高。
 * @param callback 当 OpenGL 渲染上下文创建完毕时的回调。
 */
void startGlWindowProgram(
        HINSTANCE hInstance,
        WNDPROC glWindowProc,
        int windowWidth,
        int windowHeight,
        WindowProgramCallback callback
) {
    openLog();
    logDebug("Window Enter.");

    // 1：创建窗口
    HWND hwnd = createWindow(hInstance, glWindowProc, windowWidth, windowHeight);
    // 2：创建并绑定 OpenGL 上下文
    HGLRC hGlRC = makeGLRenderEnv(hwnd);

    callback(hwnd, hGlRC);

    // 3：把窗口展示出来
    ShowWindow(hwnd, SW_SHOW);
    // 4：开始消息循环
    startWindowMessageLoop();

    logDebug("Window Exit.");
    closeLog();
}