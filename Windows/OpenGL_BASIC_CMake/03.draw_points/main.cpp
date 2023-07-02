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
    glColor4ub(255, 255, 255, 255);//预设一个颜色，每次画都会取当前设置的颜色。
    glPointSize(20.0F);//设置点的大小。

    // 开始画
    glBegin(GL_POINTS);//GL_POINTS 表述画点
    /*
     * 三个参数分别表示 x，y，z 方向上帝坐标，这个坐标是一个右手坐标系，右手握拳：
     *
     * - 中指为 z 轴，指向自己；
     * - 大拇为 x 轴，指向右边；
     * - 食指为 y 轴，指向上方。
     *
     * 由于下面 glMatrixMode 设置的是透视投影矩阵，因此绘制的内容有近大远小的现象。
     */
    glVertex3f(-5.0F, 0.0F, -10.0F);
    glVertex3f(0.0F, 0.0F, -10.0F);
    glVertex3f(5.0F, 0.0F, -10.0F);
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
    /*
     * 没有下面的矩阵，就没有办法将模型绘制到屏幕上。glMatrixMode 函数用于指定当前矩阵操作模式。OpenGL 中有三种矩阵操作模式，分别是：
     *
     *  - 模型视图矩阵 (GL_MODELVIEW)
     *  - 投影矩阵 (GL_PROJECTION)
     *  - 纹理矩阵 (GL_TEXTURE)。
     *
     * 可以通过 glMatrixMode 函数来切换这三种矩阵模式。
     */
    // 该函数告诉 GPU 渲染器，后续的矩阵操作将作用于投影矩阵，也就是我们要设置的矩阵类型。（投影矩阵一般用于 3D 绘制）
    glMatrixMode(GL_PROJECTION);
    /*
     * 这行代码设置了投影矩阵的具体参数，其中包括视场角的大小、视口的宽高比、近裁剪面和远裁剪面的位置。这些参数将决定如何将 3D 场景投射到 2D 屏幕上。该方法接受四个参数，分别是：
     *
     *  - fovy：视角的大小，以度数为单位。这个参数指定了在垂直方向上可见的视角大小，通常为 45 度到 90 度之间。
     *  - aspect：视口的宽高比，即屏幕的宽度与高度之比。这个参数指定了投影平面的宽高比，通常设置为窗口的宽高比。
     *  - zNear：近裁剪面的距离。这个参数指定了视体积的前面界限，也就是相机可以看到的最近的距离。
     *  - zFar：远裁剪面的距离。这个参数指定了视体积的后面界限，也就是相机可以看到的最远的距离。
     */
    gluPerspective(50.0F, 800.0F / 600.0F, 0.1F, 1000.0F);
    // 该函数告诉 GPU 渲染器，后续的矩阵操作将作用于模型矩阵，也就是控制模型的位置、旋转、缩放等变换的矩阵类型。
    glMatrixMode(GL_MODELVIEW);
    // 该函数将当前选择的矩阵（这里是模型矩阵）重置为单位矩阵，也就是没有任何变换的矩阵。这是为了确保我们在进行模型变换之前，不会受到之前的变换的影响。
    // 这里设置模型视口矩阵为单位矩阵，因此不会对绘制的坐标有任何影响，因此绘制的时候，视觉坐标系的坐标是怎样的，转换到视口坐标系就是怎样的。
    glLoadIdentity();

    /*
     * 总的来说，上面这段代码的作用是设置投影矩阵和模型矩阵，为后续的 3D 模型绘制做好准备。通过设置投影矩阵，我们可以控制模型在屏幕上的显示效果；
     * 通过设置模型矩阵，我们可以控制模型的位置、旋转、缩放等变换，使其呈现出我们想要的效果。
     */
}

/** Windows 桌面程序的 Main 方法 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    startGlWindowProgram(hInstance, GLWindowProc, 800, 600, ProgramCallback);
    return 0;
}