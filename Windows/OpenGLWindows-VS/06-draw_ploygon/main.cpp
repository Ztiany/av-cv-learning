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

	/*
	GL_CULL_FACE 表示只展示正面，哪个是正面呢？
		1. 取决于一个属性，ccw 还是 cw。
		2. OpenGL 默认是 ccw 的，即 counter clock wind. 即逆时针方向的。
		3. 意思就是就是摄像机所对的方向，逆时针绘制的图形的正面。

	OpenGL 默认两个面都画，设置了 GL_CULL_FACE 可以提高效率，这样就不用绘制两面，只绘制正面。
	*/
	glEnable(GL_CULL_FACE);

	//点的大小
	glPointSize(10.0F);
	//点模式
	glPolygonMode(GL_FRONT, GL_POINT);
	//线框模式【描边模式】
	//glPolygonMode(GL_FRONT, GL_LINE);
	//填充模式【默认】
	//glPolygonMode(GL_FRONT, GL_FILL);

	//画圆点，而不是画正方形点。
	glEnable(GL_POINT_SMOOTH);
	//有的机器上还需要设置这个才能画圆点，最终还是取决于硬件。
	glEnable(GL_BLEND);

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

		glBegin(GL_QUADS);//画四边形，每四个点画一个四边形。

		glColor4ub(255, 0, 0, 255);
		glVertex3f(0.0f, 0.0f, -10.0f);
		glColor4ub(0, 0, 255, 255);
		glVertex3f(-5.0f, -2.0f, -10.0f);
		glColor4ub(0, 255, 0, 255);
		glVertex3f(-3.0f, -3.0f, -10.0f);
		glColor4ub(0, 55, 100, 255);
		glVertex3f(2.0F, -2.0F, -10.0F);

		glColor4ub(255, 0, 0, 255);
		glVertex3f(5.0f, 0.0f, -10.0f);
		glColor4ub(0, 0, 255, 255);
		glVertex3f(0.0f, -2.0f, -10.0f);
		glColor4ub(0, 255, 0, 255);
		glVertex3f(2.0f, -3.0f, -10.0f);
		glColor4ub(0, 55, 100, 255);
		glVertex3f(7.0F, -2.0F, -10.0F);

		glEnd();

		//同样，还有其他模式
		//GL_QUAD_STRIP：可以绘制 （n/2 -1） 个四边形。
		//GL_PLOYGON：绘制多边形，依次将所有的点连起来，只有一个要求，这个多边形必须是凸的，不能是凹的。
		
		//present scene.
		SwapBuffers(dc);//其实就是交换缓冲区，将后缓冲区展示到前台。
	}

	OutputDebugString(L"Window Exit.");

	return 0;
}