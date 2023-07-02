#include "WindowKit.h"

HWND createWindow(
        HINSTANCE hInstance,
        WNDPROC glWindowProc,
        int windowWidth,
        int windowHeight
) {
    //-----------------------------------
    // step1：注册窗口。
    //-----------------------------------
    WNDCLASSEX wndclass;
    wndclass.cbClsExtra = 0;
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.cbWndExtra = 0;
    wndclass.hbrBackground = nullptr;
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass.hIcon = nullptr;
    wndclass.hIconSm = nullptr;
    wndclass.hInstance = hInstance;
    wndclass.lpfnWndProc = glWindowProc;
    wndclass.lpszClassName = "GLWindow";
    wndclass.lpszMenuName = nullptr;
    wndclass.style = CS_VREDRAW | CS_HREDRAW;
    ATOM atom = RegisterClassEx(&wndclass);
    if (!atom) {
        ExitProcess(0);
    }

    //-----------------------------------
    // step2：创建窗口。
    //-----------------------------------
    HWND hwnd = CreateWindowEx(
            NULL,
            "GLWindow",
            "OpenGL Window",
            WS_OVERLAPPEDWINDOW,
            100, 100,
            windowWidth, windowHeight,
            nullptr,
            nullptr,
            hInstance,
            nullptr
    );

    return hwnd;
}

void startWindowMessageLoop() {
    MSG msg;
    while (true) {
        if (PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            //转换消息，并分发给自己。
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}