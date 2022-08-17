#include <tchar.h>
#include <windows.h>

/* 消息处理函数 */
LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_SIZE:
            break;
        //处理退出消息
        case WM_CLOSE:
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

/*
 * 入口函数
 *
 * @hInstance 可执行程序句柄
 * @hPrevInstance 上一个可执行程序句柄
 * @lpCmdLine 程序参数
 * @nShowCmd 是否显示窗口
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // 1   注册窗口类
    ::WNDCLASSEXA winClass;
    winClass.lpszClassName = "Raster"; //指定窗口的名称
    winClass.cbSize = sizeof(::WNDCLASSEX);
    winClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    winClass.lpfnWndProc = windowProc;
    winClass.hInstance = hInstance;
    winClass.hIcon = 0;
    winClass.hIconSm = 0;
    winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)(BLACK_BRUSH);
    winClass.lpszMenuName = NULL;
    winClass.cbClsExtra = 0;
    winClass.cbWndExtra = 0;
    RegisterClassExA(&winClass);

    /*
     2 创建窗口：
         CreateWindowEx 对应的宽字符集函数为 CreateWindowExA。
         可以右键项目-->属性-->字符集-->选择“使用多字符集”，这样就不需要使用 CreateWindowEx 函数了
     */
    HWND hWnd = CreateWindowEx(
        //句柄
        NULL,
        //用于查找上面注册的窗口
        "Raster",
        //窗口标题
        "Raster",
        //窗口风格
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        //窗口的位置和大小
        0, 0, 480, 320, 0, 0,
        //句柄
        hInstance,
        //用户自定义的变量，给 0 即可
        0);

    UpdateWindow(hWnd);
    ShowWindow(hWnd, SW_SHOW); //执行到这里，就显示出来了

    // 3 展示一张图片
    RECT rt = {0};
    GetClientRect(hWnd, &rt); //获取用户区（除标题栏的其他区域）

    int width = rt.right - rt.left;
    int height = rt.bottom - rt.top;

    //画板
    HDC hDC = GetDC(hWnd);
    HDC hMem = ::CreateCompatibleDC(hDC);

    // 声明位图【纸张】
    BITMAPINFO bmpInfor;
    void* buffer = 0; //图片缓冲区

    bmpInfor.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfor.bmiHeader.biWidth = width;
    bmpInfor.bmiHeader.biHeight = height;
    bmpInfor.bmiHeader.biPlanes = 1;
    bmpInfor.bmiHeader.biBitCount = 32; //一个像素 32 位
    bmpInfor.bmiHeader.biCompression = BI_RGB;
    bmpInfor.bmiHeader.biSizeImage = 0;
    bmpInfor.bmiHeader.biXPelsPerMeter = 0;
    bmpInfor.bmiHeader.biYPelsPerMeter = 0;
    bmpInfor.bmiHeader.biClrUsed = 0;
    bmpInfor.bmiHeader.biClrImportant = 0;

    // 创建一张图片
    HBITMAP hBmp = CreateDIBSection(hDC, &bmpInfor, DIB_RGB_COLORS, (void**)&buffer, 0, 0);
    // 将画板和纸张关联【Windows 绘图不能直接画在窗口上， 必须先在画板上绘制】
    SelectObject(hMem, hBmp);

    //将 buffer 中的数据都置为 0
    memset(buffer, 0, width * height * 4);

    // 4 进入循环，防止窗口退出
    MSG msg = {0};
    while (true)
    {
        if (msg.message == WM_DESTROY || msg.message == WM_CLOSE || msg.message == WM_QUIT)
        {
            break;
        }
        // PeekMessage 用于从消息队列中取出消息
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            // DispatchMessage 消息后，就会调用上面注册的 windowProc 函数
            DispatchMessage(&msg);
        }

        //修改缓冲区中的一部分像素点
        memset(buffer, 0, width * height * 4);
        unsigned char* rgba = (unsigned char*)buffer;
        int pitch = width * 4;
        memset(rgba + pitch * 10, 255, pitch);

        // The BitBlt function performs a bit-block transfer of the color data corresponding to a
        // rectangle of pixels from the specified source device context into a destination device
        // context.
        BitBlt(hDC, 0, 0, width, height, hMem, 0, 0, SRCCOPY);
    }

    return 0;
}
