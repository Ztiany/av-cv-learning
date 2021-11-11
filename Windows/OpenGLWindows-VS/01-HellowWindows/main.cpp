#include <Windows.h>

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
		//窗口注册名，与 wndclass.lpszClassName 保持一致
		L"GLWindow",
		//窗口名
		L"OpenGL Window", WS_OVERLAPPEDWINDOW,
		//窗口位置
		100, 100,
		//窗口大小
		800, 600,
		//其他参数
		NULL, NULL, hInstance, NULL);
	//step3：展示窗口
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	//防止串口给退出，开始监听事件
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
	}

	OutputDebugString(L"Window Exit.");

	return 0;
}