#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <LogKit.h>
#include <GLKit.h>

void glRender(HWND hwnd) {
    //清空画布
    glClearColor(0.1, 0.4, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // 设置绘制属性
    glPointSize(10.0F);

    // 重置矩阵
    glLoadIdentity();

    // 利用矩阵操作坐标
    glScaled(0.5F, 0.5F, 2.0F);

    // 开始画
    glBegin(GL_POLYGON);
    glVertex3f(-3.0f, 5.0f, -15.0f);
    glVertex3f(-5.0f, 2.0f, -15.0f);
    glVertex3f(-3.0f, -2.0f, -15.0f);
    glVertex3f(0.0F, -5.0F, -15.0F);
    glVertex3f(3.0F, -4.0F, -15.0F);
    glVertex3f(2.0F, 0.0F, -15.0F);
    glEnd();
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
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            glRender(hwnd);
            SwapBuffers(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        default: {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
}

VOID CALLBACK ProgramCallback(HWND hwnd, HGLRC hGLRC) {
    // 设置投影
    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0F, 800.0F / 600.0F, 0.1F, 1000.0F);
    glMatrixMode(GL_MODELVIEW);
}

/** Windows 桌面程序的 Main 方法 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    startGlWindowProgram(hInstance, GLWindowProc, 800, 600, ProgramCallback);
    return 0;
}