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
    glColor4ub(255, 0, 0, 255);//set current color: white. 预设一个颜色，每次画都会取当前设置的颜色。

    // 开始画
    // 顺时针方向
    glBegin(GL_TRIANGLES);
    glVertex3f(-5.0F, 4.0F, -10.0F);
    glVertex3f(-2.0F, 1.0F, -10.0F);
    glVertex3f(-5.0F, 1.0F, -10.0F);
    glEnd();

    // 逆时针方向
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0F, 4.0F, -10.0F);
    glVertex3f(0.0F, 1.0F, -10.0F);
    glVertex3f(3.0F, 1.0F, -10.0F);
    glEnd();

    // GL_TRIANGLE_STRIP：该模型下，奇数个点与偶数个点的连线方式不同。
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-5.0F, -1.0F, -10.0F);
    glColor4ub(0, 255, 0, 255);
    glVertex3f(-2.0F, -4.0F, -10.0F);
    glColor4ub(0, 0, 250, 255);
    glVertex3f(-5.0F, -4.0F, -10.0F);
    glColor4ub(10, 10, 50, 255);
    glVertex3f(-5.0F, -4.5F, -10.0F);
    glEnd();

    // GL_TRIANGLE_FAN：该模式，总是从第一个点开始。
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0F, -1.0F, -10.0F);
    glColor4ub(0, 255, 0, 255);
    glVertex3f(3.0F, -4.0F, -10.0F);
    glColor4ub(0, 0, 250, 255);
    glVertex3f(0.0F, -4.0F, -10.0F);
    glColor4ub(110, 10, 50, 255);
    glVertex3f(-2.0F, -2.5F, -10.0F);
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

    /*
     *  我们可以顺时针方向画图形，也可以逆时针方向画图形。OpenGL 默认会把两个方向的图形都画出来。
     *
     *  设置了 GL_CULL_FACE 可以提高效率，这样就不用绘制两面，只绘制正面。GL_CULL_FACE 表示只展示正面，哪个是正面呢？
     *
     *      1. 取决于一个属性，ccw 还是 cw。
     *      2. OpenGL 默认是 ccw 的，即 counter clock wind. 即逆时针方向的。
     *      3. 意思就是就是摄像机所对的方向，逆时针绘制的图形的正面。
     */
    glEnable(GL_CULL_FACE);
    //通过修改 front face 为 GL_CW，此时摄像机所对的方向，顺时针绘制的图形就是正面。
    glFrontFace(GL_CW);
}

/** Windows 桌面程序的 Main 方法 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    startGlWindowProgram(hInstance, GLWindowProc, 800, 600, ProgramCallback);
    return 0;
}