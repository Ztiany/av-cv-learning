#include <tchar.h>
#include <windows.h>

/**
 * 窗口事件回调函数。
 *
 * @param hwnd 指定当前窗口的句柄。HWND 是一个句柄类型，用于表示 Windows 窗口的句柄，它是一个唯一标识符，用于识别窗口。
 * @param msg 指定当前消息的类型。UINT 是一个无符号整数类型，用于表示 Windows 消息的类型，例如键盘事件、鼠标事件、绘图事件等。
 * @param wParam 指定消息的附加信息。WPARAM 是一个无符号整数类型，用于表示与消息相关的附加信息，例如按下的键码、鼠标的位置等。
 * @param lParam 指定消息的附加信息。LPARAM 是一个长整数类型，用于表示与消息相关的附加信息，例如鼠标点击的时间戳、窗口的坐标等。
 * @return LRESULT CALLBACK 指定该函数的返回值类型。LRESULT 是一个长整数类型，用于表示函数的返回值，通常用于返回处理结果或错误代码。CALLBACK 是一个宏定义，用于指定该函数是一个回调函数。
 */
LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE:
            break;
        case WM_CLOSE:
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void registerWindow(HINSTANCE hInstance, WNDCLASSEXA &winClass) {
    winClass.lpszClassName = "Raster"; //指定窗口的名称
    winClass.cbSize = sizeof(WNDCLASSEX);
    winClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    winClass.lpfnWndProc = windowProc;
    winClass.hInstance = hInstance;
    winClass.hIcon = nullptr;
    winClass.hIconSm = nullptr;
    winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH) (BLACK_BRUSH);
    winClass.lpszMenuName = nullptr;
    winClass.cbClsExtra = 0;
    winClass.cbWndExtra = 0;
    RegisterClassExA(&winClass);
}

/**
 * 入口函数
 *
 * @hInstance 可执行程序句柄
 * @hPrevInstance 上一个可执行程序句柄
 * @lpCmdLine 程序参数
 * @nShowCmd 是否显示窗口
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // 1   注册窗口类
    WNDCLASSEXA winClass;
    registerWindow(hInstance, winClass);

    /*
     2 创建窗口：CreateWindowEx 对应的宽字符集函数为 CreateWindowExA。
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
            0, 0, 480, 320,
            //parent
            nullptr,
            //菜单
            nullptr,
            //句柄
            hInstance,
            //用户自定义的变量，给 0 即可
            nullptr
    );

    UpdateWindow(hWnd);
    ShowWindow(hWnd, SW_SHOW); //执行到这里，窗口就显示出来了

    // 3 展示一张图片
    RECT rt = {0};
    GetClientRect(hWnd, &rt); //获取用户区（除标题栏的其他区域）

    int width = rt.right - rt.left;
    int height = rt.bottom - rt.top;

    //画板
    HDC hDC = GetDC(hWnd);

    /*
     * 为什么还需要一个内存设备上下文句柄，有什么区别？
     *
     * 使用内存设备上下文（MemDC）的主要目的是为了避免直接在屏幕上绘制位图时产生闪烁的问题，同时也可以提高绘制效率。
     *
     * 在 Windows 系统中，所有的绘图操作都是通过设备上下文（DC）来完成的。设备上下文是一个句柄，它提供了一组函数，可以用于在指定的设备上进行绘图操作。
     * 通常情况下，设备上下文是与屏幕或打印机等物理设备相关联的，它直接操作的是物理设备的显存或打印机缓冲区等。
     *
     * 但是，直接在屏幕上绘制位图时，由于需要不断地修改显存中的像素数据，因此可能会产生闪烁的问题。为了避免这个问题，可以使用内存设备上下文（MemDC），
     * 它是一个虚拟的设备上下文，不直接与物理设备相关联，而是在内存中进行绘图操作。
     *
     * 使用内存设备上下文的流程是：
     *
     *      1. 创建一个内存设备上下文句柄MemDC。
     *      2. 创建一个位图对象，并将其与内存设备上下文关联。
     *      3. 在内存设备上下文中绘制位图或其他图形。
     *      4. 将内存设备上下文中的内容复制到屏幕或其他物理设备上。
     *
     * 这样，就可以避免直接在屏幕上绘制位图时产生闪烁的问题，同时也可以提高绘制效率，因为在内存中进行绘图操作比直接在屏幕上绘制要快得多。
     */
    HDC hMem = ::CreateCompatibleDC(hDC);

    /*
     * 声明位图【纸张】
     *
     * 在 Win32 API 中，BITMAPINFO 结构体用于描述设备无关位图（DIB）的格式和颜色信息。BITMAPINFO 结构体包含了 BITMAPINFOHEADER 结构体和一个颜色表数组，用于描述位图的
     * 格式和颜色信息。
     *
     * BITMAPINFOHEADER 结构体包含了位图的头信息，包括位图宽度、高度、颜色深度、压缩方式、位图大小等信息。颜色表数组包含了位图的颜色信息，不同颜色深度的位图，颜色表的结构也不相同。
     *
     */
    BITMAPINFO bmpInfo;
    void *buffer = nullptr; //图片缓冲区

    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);//该结构体大小，用于确定该结构体的字节数。
    bmpInfo.bmiHeader.biWidth = width;//位图的宽度（像素数）。
    bmpInfo.bmiHeader.biHeight = height;//位图的高度（像素数）。如果该值为正数，则表示位图从下往上排列（底部在先），如果该值为负数，则表示位图从上往下排列（顶部在先）。
    bmpInfo.bmiHeader.biPlanes = 1;//位图的平面数，必须为1。
    bmpInfo.bmiHeader.biBitCount = 32; //每个像素所占的位数，即颜色深度，可以是 1、4、8、16、24 或 32 位。
    bmpInfo.bmiHeader.biCompression = BI_RGB;//指定位图的压缩方式，可以是 BI_RGB（不压缩）、BI_RLE8（8 位 RLE 压缩）或 BI_RLE4（4 位 RLE 压缩）。
    bmpInfo.bmiHeader.biSizeImage = 0;//位图实际大小，以字节为单位。如果压缩方式为 BI_RGB，则该值可以设置为 0，系统会自动计算位图大小。
    bmpInfo.bmiHeader.biXPelsPerMeter = 0;//位图水平分辨率，以每米像素数为单位。
    bmpInfo.bmiHeader.biYPelsPerMeter = 0;//位图垂直分辨率，以每米像素数为单位。
    bmpInfo.bmiHeader.biClrUsed = 0;//位图调色板中实际使用的颜色数，如果为 0，则表示使用所有颜色。
    bmpInfo.bmiHeader.biClrImportant = 0;//位图调色板中对图像显示有重要影响的颜色数，如果为 0，则表示所有颜色都重要。

    // 创建一张图片
    HBITMAP hBmp = CreateDIBSection(hDC, &bmpInfo, DIB_RGB_COLORS, (void **) &buffer, nullptr, 0);
    // 将画板和纸张关联【Windows 绘图不能直接画在窗口上， 必须先在画板上绘制】
    SelectObject(hMem, hBmp);

    //将 buffer 中的数据都置为 0
    memset(buffer, 0, width * height * 4);

    // 4 进入循环，防止窗口退出
    MSG msg = {nullptr};
    while (true) {
        if (msg.message == WM_DESTROY || msg.message == WM_CLOSE || msg.message == WM_QUIT) {
            break;
        }
        // PeekMessage 用于从消息队列中取出消息
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            // DispatchMessage 消息后，就会调用上面注册的 windowProc 函数
            DispatchMessage(&msg);
        }

        //修改缓冲区中的一部分像素点
        memset(buffer, 0, width * height * 4);
        auto *rgba = (unsigned char *) buffer;
        int pitch = width * 4;
        memset(rgba + pitch * 10, 255, pitch);

        // The BitBlt function performs a bit-block transfer of the color data corresponding to a
        // rectangle of pixels from the specified source device context into a destination device
        // context.
        BitBlt(hDC, 0, 0, width, height, hMem, 0, 0, SRCCOPY);
    }

    return 0;
}