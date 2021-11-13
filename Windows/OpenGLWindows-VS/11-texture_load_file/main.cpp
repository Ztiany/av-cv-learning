//Windows 库
#include <Windows.h>

//OpenGL 库
#include <gl/GL.h>
#include <gl/GLU.h>

//自定的封装
#include "Texture.h"
#include "utils.h"

//指定需要 OpenGL 的库【只要装了 VS 就会有这些库】
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

//回调函数：wndclass.lpfnWndProc
LRESULT CALLBACK GLWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/**Windows 桌面程序的 Main 方法*/
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	OutputDebugString(L"Window Enter.");

	//-----------------------------------
	// step1：注册窗口。
	//-----------------------------------
	WNDCLASSEX wndclass;//窗口的属性。
	wndclass.cbClsExtra = 0;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = NULL;
	wndclass.hIconSm = NULL;
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = GLWindowProc;//事件回调，比如关闭窗口。
	wndclass.lpszClassName = L"GLWindow";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_VREDRAW | CS_HREDRAW;
	ATOM atom = RegisterClassEx(&wndclass);//执行注册。
	if (!atom) {
		MessageBox(NULL, L"Register Fail", L"Error", MB_OK);
		return 0;
	}

	//-----------------------------------
	// step2：创建窗口。
	//-----------------------------------
	HWND hwnd = CreateWindowEx(
		NULL,
		//窗口注册名，与 wndclass.lpszClassName 保持一致。
		L"GLWindow",
		//窗口名。
		L"OpenGL Window", WS_OVERLAPPEDWINDOW,
		//窗口位置。
		100, 100,
		//窗口大小。
		800, 600,
		//其他参数。
		NULL, NULL, hInstance, NULL);

	//-----------------------------------
	// opengl：create opengl render context.
	//-----------------------------------
	HDC dc = GetDC(hwnd);//获取设备，OpenGL 需要这个。
	PIXELFORMATDESCRIPTOR pfd;//颜色描述符。
	pfd.nVersion = 1;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.cColorBits = 32;//颜色。
	pfd.cDepthBits = 24;//深度。
	pfd.cStencilBits = 8;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags =
		PFD_DRAW_TO_WINDOW //画到窗口上。
		| PFD_SUPPORT_OPENGL //颜色要支持 OpenGL。
		| PFD_DOUBLEBUFFER; //支持双缓冲。

	//调用函数根据设置的属性去选择颜色描述符。
	int pixelFormat = ChoosePixelFormat(dc, &pfd);
	//选择好就设置进去。
	SetPixelFormat(dc, pixelFormat, &pfd);
	//create context.
	HGLRC rc = wglCreateContext(dc);
	wglMakeCurrent(dc, rc);//rc 和 dc 作为当前的渲染
	//setup opengl context complete.

	// 预设一个颜色。
	//【glClearColor：glClearColor specifies the red, green, blue, and alpha values used by glClear to clear the color buffers. Values specified by glClearColor are clamped to the range 0 1 .】
	glClearColor(0.1, 0.4, 0.6, 1.0);//set "clear color" for background.

	//-----------------------------------
	// opengl：init
	//-----------------------------------
	//没有下面的矩阵，就没有办法将模型绘制到屏幕上。
	glMatrixMode(GL_PROJECTION);//tell the gpu render that I would select the projection matrix. 投影矩阵
	gluPerspective(50.0F, 800.0F / 600.0F, 0.1F, 1000.0F);//set some values to projection matrix. 透视矩阵
	glMatrixMode(GL_MODELVIEW);//tell the gpu render that I would select the model view matrix. 模型矩阵
	glLoadIdentity();//给选择的矩阵传一个单位矩阵，因为 glLoadIdentity 调用之前选择的是模型矩阵（调用了 glMatrixMode），所以这个调用作用于模型矩阵。

	//测试 LoadFileContent
	char* content = (char*)LoadFileContent("test.txt");
	// 为了展示打印内容，需要一个命令窗口，在项目属性-->生成事件-->生成后事件-->命令行 中，添加下面内容。
	//editbin /subsystem:console $(OutDir)$(ProjectName).exe
	printf("content: %s", content);

	//纹理
	Texture texture;
	texture.Init("test.bmp");
	

	//-----------------------------------
	// step3：展示窗口。
	//-----------------------------------
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	//-----------------------------------
	// step4：防止窗口退出，循环地监听事件。
	//-----------------------------------
	MSG msg;//用于接收消息
	while (true) {//这个 while 的执行跟刷新率有关系，比如 60hz 的刷新率每秒跑 60 次。
		//接收消息
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			//转换消息，并分发给自己。
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//-----------------------------------
		// opengl：draw and present.
		//-----------------------------------
		//draw sence.

		//【glClear：clear buffers to preset values.】
		glClear(GL_COLOR_BUFFER_BIT);//GL_COLOR_BUFFER_BIT 表示擦除的是颜色缓冲区，就是用上面 glClearColor 设置的颜色来擦除。

		//启动并绑定纹理
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture.mTextureID);
		//纹理坐标：左下脚为原点，范围 [0, 1]
		glTexCoord2f(0.5F, 1.0F);

		glBegin(GL_TRIANGLES);//start to draw something
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-1.0f, -0.5f, -2.0f);//左下角

		glTexCoord2f(2.0f, 0.0f);
		glVertex3f(1.0f, -0.5f, -2.0f);//右下角

		glTexCoord2f(0.0, 2.0f);
		glVertex3f(-1.0f, -0.5f, -3.0f);//左下角

		glTexCoord2f(2.0f, 0.0f);
		glVertex3f(1.0f, -0.5f, -2.0f);//右下角

		glTexCoord2f(2.0f, 2.0f);
		glVertex3f(1.0f, -0.5f, -3.0f);//右下角

		glTexCoord2f(0.0, 2.0f);
		glVertex3f(-1.0f, -0.5f, -3.0f);//左下角

		glEnd();//end

		//present scene.
		SwapBuffers(dc);//其实就是交换缓冲区，将后缓冲区展示到前台。
	}

	OutputDebugString(L"Window Exit.");

	return 0;
}