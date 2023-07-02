#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <LogKit.h>
#include <GLKit.h>

void glRender(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    glClearColor(0.1, 0.4, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // 设置画笔
    glLineWidth(4.0F);

    // 开始画
    // 模式1 GL_LINES：两两成线，多余的点丢弃。【具体实现依赖于硬件】
    glBegin(GL_LINES);
    glColor4ub(255, 0, 0, 255);//预设一个颜色，每次画都会取当前设置的颜色。
    glVertex3f(-5.0F, 4.0F, -10.0F);
    glVertex3f(-2.0F, 1.0F, -10.0F);

    glVertex3f(-2.0F, 1.0F, -10.0F);
    glVertex3f(-5.0F, 1.0F, -10.0F);
    glEnd();//drawing end. corresponding with glBegin.

    // 模式2 GL_LINE_LOOP：首尾相连。
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0F, 4.0F, -10.0F);
    glVertex3f(3.0F, 1.0F, -10.0F);
    glVertex3f(0.0F, 1.0F, -10.0F);
    glEnd();

    // 模式3 GL_LINE_STRIP：多点成线，不会自动封闭。
    glBegin(GL_LINE_STRIP);
    glColor4ub(255, 0, 0, 255);//预设一个颜色，每次画都会取当前设置的颜色。
    glVertex3f(0.0F, -1.0F, -10.0F);

    glColor4ub(0, 255, 0, 255);//change current color.【会画出一条渐变的线，从前一个颜色色插值到现在的颜色】
    glVertex3f(3.0F, -4.0F, -10.0F);

    glColor4ub(0, 0, 255, 255);//change current color.【会画出一条渐变的线，从前一个颜色色插值到现在的颜色】
    glVertex3f(0.0F, -4.0F, -10.0F);
    glEnd();//drawing end. corresponding with glBegin.

    SwapBuffers(hdc);
    EndPaint(hwnd, &ps);
}

/** 回调函数 */
LRESULT CALLBACK GLWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_PAINT: {
            logDebug("GLWindowProc: WM_PAINT");
            glRender(hwnd);
            return 0;
        }
        default: {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
}

VOID CALLBACK ProgramCallback(HWND hwnd, HGLRC hGLRC) {
    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0F, 800.0F / 600.0F, 0.1F, 1000.0F);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/** Windows 桌面程序的 Main 方法 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    startGlWindowProgram(hInstance, GLWindowProc, 800, 600, ProgramCallback);
    return 0;
}