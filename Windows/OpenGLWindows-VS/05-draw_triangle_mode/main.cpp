#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

LRESULT CALLBACK GLWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//register window
	WNDCLASSEX wndclass;
	wndclass.cbClsExtra = 0;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = NULL;
	wndclass.hIconSm = NULL;
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = GLWindowProc;
	wndclass.lpszClassName = L"GLWindow";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_VREDRAW | CS_HREDRAW;
	ATOM atom = RegisterClassEx(&wndclass);
	if (!atom)
	{
		return 0;
	}
	//create window
	HWND hwnd = CreateWindowEx(NULL, L"GLWindow", L"OpenGL Window", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);
	//create opengl render context
	HDC dc = GetDC(hwnd);
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nVersion = 1;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	int pixelFormat = ChoosePixelFormat(dc, &pfd);
	SetPixelFormat(dc, pixelFormat, &pfd);

	HGLRC rc = wglCreateContext(dc);
	wglMakeCurrent(dc, rc);//setup opengl context complete
	//opengl init
	glMatrixMode(GL_PROJECTION);//tell the gpu processer that i would select the projection matrix
	gluPerspective(50.0f, 800.0f / 600.0f, 0.1f, 1000.0f);//set some values to projection matrix
	glMatrixMode(GL_MODELVIEW);//tell .... model view matrix
	glLoadIdentity();

	glClearColor(0.1f, 0.4f, 0.6f, 1.0f);//set "clear color" for background
	//show window
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	glEnable(GL_CULL_FACE);
	//front face : ccw -> counter clock wind 
	MSG msg;
	while (true)
	{
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//draw scene
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_TRIANGLE_FAN);//start to draw something n-2

		glColor4ub(255, 0, 0, 255);
		glVertex3f(0.0f, 0.0f, -10.0f);

		glColor4ub(0, 0, 255, 255);
		glVertex3f(-5.0f, -2.0f, -10.0f);

		glColor4ub(0, 255, 0, 255);
		glVertex3f(-3.0f, -2.0f, -10.0f);

		glColor4ub(0, 0, 255, 255);
		glVertex3f(-1.0f, -2.0f, -10.0f);

		glColor4ub(0, 255, 255, 255);
		glVertex3f(2.0f, -2.0f, -10.0f);

		glEnd();//end

		//present scene
		SwapBuffers(dc);
	}

	return 0;
}