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

    // 矩阵入栈（类似图层的概念）
    glLoadIdentity();

    // 开始画
    glBegin(GL_TRIANGLES);

    glNormal3f(0.0F, -1.0F, 0.0F);//设置法线
    glColor4ub(0, 255, 0, 255);
    glVertex3f(-1.0F, -0.5F, -2.0F);

    glNormal3f(0.0F, 1.0F, 0.0F);//设置法线
    glColor4ub(0, 0, 250, 255);
    glVertex3f(1.0F, -0.5F, -2.0F);

    glNormal3f(0.0F, 1.0F, 0.0F);//设置法线
    glColor4ub(110, 10, 50, 255);
    glVertex3f(0.0F, -0.5F, -10.0F);//画一个更远的点

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

    // 设置为单位矩阵
    glLoadIdentity();

    // 设置光照（固定管线中只有 8 盏灯）
    float blackColor[] = {0.0F,0.0F,0.0F,1.0F};
    float whiteColor[] = {1.0F,1.0F,1.0F,1.0F};
    float lightPosition[] = {0.0F,1.0F,0.0F,0.0F};//方向光（这是一个齐次坐标，最后一个为 0，表示为一个无穷远处的光）
    glLightfv(GL_LIGHT0, GL_AMBIENT, whiteColor);//环境光
    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteColor);//漫反射光（与法线有关）
    glLightfv(GL_LIGHT0, GL_SPECULAR, whiteColor);//镜面反射光（与法线有关）
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // 设置材质
    float  blackMat[] = {0.0F,0.0F,0.0F,1.0F};
    float  ambientMat[] = {0.1F,0.1F,0.1F,1.0F};
    float  diffuseMat[] = {0.4F,0.4F,0.4F,1.0F};
    float  specularMat[] = {0.9F,0.9F,0.9F,1.0F};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMat);//环境光
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMat);//漫反射光
    glMaterialfv(GL_FRONT, GL_SPECULAR, specularMat);//镜面反射光

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);//0 号灯光
}

/** Windows 桌面程序的 Main 方法 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    startGlWindowProgram(hInstance, GLWindowProc, 800, 600, ProgramCallback);
    return 0;
}