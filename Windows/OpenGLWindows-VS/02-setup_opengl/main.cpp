//Windows 库
#include <Windows.h>

//OpenGL 库
#include <gl/GL.h>
#include <gl/GLU.h>

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

	//step1：注册窗口
	WNDCLASSEX wndclass;//窗口的属性
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
	ATOM atom = RegisterClassEx(&wndclass);//执行注册
	if (!atom) {
		MessageBox(NULL, L"Register Fail", L"Error", MB_OK);
		return 0;
	}

	//step2：创建窗口
	HWND hwnd = CreateWindowEx(
		NULL,
		//窗口注册名，与 wndclass.lpszClassName 保持一致。
		L"GLWindow",
		//窗口名
		L"OpenGL Window", WS_OVERLAPPEDWINDOW,
		//窗口位置
		100, 100,
		//窗口大小
		800, 600,
		//其他参数
		NULL, NULL, hInstance, NULL);

	//opengl1：create opengl render context.【不需要太关注】
	HDC dc = GetDC(hwnd);//获取设备，OpenGL 需要这个。
	PIXELFORMATDESCRIPTOR pfd;//颜色描述符
	pfd.nVersion = 1;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.cColorBits = 32;//颜色
	pfd.cDepthBits = 24;//深度
	pfd.cStencilBits = 8;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags =
		PFD_DRAW_TO_WINDOW //画到窗口上
		| PFD_SUPPORT_OPENGL //支持OpenGL
		| PFD_DOUBLEBUFFER; //支持双缓冲

	int pixelFormat = ChoosePixelFormat(dc, &pfd);//调用 Windows 去选择
	SetPixelFormat(dc, pixelFormat, &pfd);//选择好就设置进去
	HGLRC rc = wglCreateContext(dc);
	wglMakeCurrent(dc, rc);//setup opengl context complete.

	//opengl2：init opengl
	glClearColor(0.1, 0.4, 0.6, 1.0);//set "clear color" for background

	//step3：展示窗口
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	//防止窗口给退出，开始监听事件
	MSG msg;//用于接收消息
	while (true) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {//接收消息
			if (msg.message == WM_QUIT) {
				break;
			}
			//转换消息，并分发给自己
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//opengl3：绘制
		//draw scene
		glClear(GL_COLOR_BUFFER_BIT);
		//present scene
		SwapBuffers(dc);//其实就是交换缓冲区
	}

	OutputDebugString(L"Window Exit.");

	return 0;
}