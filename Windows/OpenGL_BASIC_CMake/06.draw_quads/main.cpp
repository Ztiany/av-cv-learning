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

    // 开始画
    //GL_QUADS 模式：画四边形，每四个点画一个四边形。
    glBegin(GL_QUADS);

    glColor4ub(255, 0, 0, 255);
    glVertex3f(-1.0f, 4.0f, -10.0f);
    glColor4ub(0, 0, 255, 255);
    glVertex3f(-6.0f, 2.0f, -10.0f);
    glColor4ub(0, 255, 0, 255);
    glVertex3f(-4.0f, 1.0f, -10.0f);
    glColor4ub(0, 55, 100, 255);
    glVertex3f(1.0F, 2.0F, -10.0F);

    glColor4ub(255, 0, 0, 255);
    glVertex3f(4.0f, 4.0f, -10.0f);
    glColor4ub(0, 0, 255, 255);
    glVertex3f(-1.0f, 2.0f, -10.0f);
    glColor4ub(0, 255, 0, 255);
    glVertex3f(1.0f, 1.0f, -10.0f);
    glColor4ub(0, 55, 100, 255);
    glVertex3f(6.0F, 1.0F, -10.0F);

    glEnd();

    //GL_QUAD_STRIP 模式：可以绘制 （n/2 -1） 个四边形。
    glBegin(GL_QUAD_STRIP);

    glColor4ub(255, 0, 0, 255);
    glVertex3f(-1.0f, 0.0f, -10.0f);
    glColor4ub(0, 0, 255, 255);
    glVertex3f(-6.0f, -2.0f, -10.0f);
    glColor4ub(0, 55, 100, 255);
    glVertex3f(1.0F, -2.0F, -10.0F);
    glColor4ub(0, 255, 0, 255);
    glVertex3f(-4.0f, -3.0f, -10.0f);
    glColor4ub(100, 155, 20, 255);
    glVertex3f(4.0f, -4.0f, -10.0f);
    glColor4ub(100, 15, 220, 255);
    glVertex3f(4.0f, -5.0f, -10.0f);

    glEnd();

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
    // 设置投影
    glMatrixMode(GL_PROJECTION);
    gluPerspective(50.0F, 800.0F / 600.0F, 0.1F, 1000.0F);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 设置点的大小
    glPointSize(10.0F);

    // 设置填充模式
    // 点模式，点与点之间不连线
    glPolygonMode(GL_FRONT, GL_POINT);
    // 描边模式，点与点之间连线
    //glPolygonMode(GL_FRONT, GL_LINE);
    // 填充模式，点与点之间连线，且填充线围成的图形
    //glPolygonMode(GL_FRONT, GL_FILL);

    // 画圆点，而不是画正方形点。
    glEnable(GL_POINT_SMOOTH);
    // 有的机器上还需要设置这个才能画圆点，最终还是取决于硬件。
    glEnable(GL_BLEND);
}

/** Windows 桌面程序的 Main 方法 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    startGlWindowProgram(hInstance, GLWindowProc, 800, 600, ProgramCallback);
    return 0;
}