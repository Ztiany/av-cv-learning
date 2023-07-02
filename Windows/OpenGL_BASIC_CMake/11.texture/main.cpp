#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <LogKit.h>
#include <GLKit.h>
#include <TextureKit.h>

Texture texture;

void glRender() {
    // 清空画布
    glClearColor(0.1, 0.4, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // 设置绘制属性
    glPointSize(10.0F);

    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, texture.mTextureID);

    // 开始画
    glBegin(GL_TRIANGLES);

    glTexCoord2f(0.0f, 0.0f);//指定纹理坐标
    glVertex3f(-1.0f, -0.5f, -2.0f);

    glTexCoord2f(2.0f, 0.0f);
    glVertex3f(1.0f, -0.5f, -2.0f);

    glTexCoord2f(0.0, 2.0f);
    glVertex3f(-1.0f, -0.5f, -3.0f);

    glTexCoord2f(2.0f, 0.0f);
    glVertex3f(1.0f, -0.5f, -2.0f);

    glTexCoord2f(2.0f, 2.0f);
    glVertex3f(1.0f, -0.5f, -3.0f);

    glTexCoord2f(0.0, 2.0f);
    glVertex3f(-1.0f, -0.5f, -3.0f);

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
            glRender();
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

    // 启动纹理
    glEnable(GL_TEXTURE_2D);

    // 加载纹理
    texture.Init("test.bmp");//init opengl texture
    logDebug("loaded texture = %d", texture.mTextureID);
}

/** Windows 桌面程序的 Main 方法 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    startGlWindowProgram(hInstance, GLWindowProc, 800, 600, ProgramCallback);
    return 0;
}