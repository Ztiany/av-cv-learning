#include <Windows.h>
#include <gl/GL.h>
#include <LogKit.h>

/** 回调函数 */
LRESULT CALLBACK GLWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_PAINT: {
            logDebug("GLWindowProc: WM_PAINT");
            // 这里使用 OpenGL 进行绘制。
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // glClearColor specifies the red, green, blue, and alpha values used by glClear to clear the color buffers.
            // Values specified by glClearColor are clamped to the range 0 1 .
            glClearColor(0.1, 0.4, 0.6, 1.0);
            /*
             * glClear() 是 OpenGL 中的一个函数，用于清空当前帧缓冲区（framebuffer）中的颜色、深度和模板缓冲区。glClear() 函数在每次渲染前都会被调用，以确保每一帧都是
             * 全新的。其 mask 参数是一个位掩码（bitmask），用于指定需要清空的缓冲区。可以使用以下常量来指定需要清空的缓冲区：
             *
             *  - GL_COLOR_BUFFER_BIT：颜色缓冲区
             *  - GL_DEPTH_BUFFER_BIT：深度缓冲区
             *  - GL_STENCIL_BUFFER_BIT：模板缓冲区
             *
             *  在实际应用中，每次渲染前都需要调用 glClear() 函数清空缓冲区，以避免上一帧的残留内容对当前帧的影响。需要注意的是，如果没有清空深度缓冲区，可能会导致深度
             *  测试（depth test）不准确，从而影响场景的深度排序。如果没有清空颜色缓冲区，可能会导致上一帧的图像残留在当前帧中，从而影响场景的真实感和清晰度。因此，清空
             *  缓冲区是 OpenGL 渲染中的一个重要步骤。
             */
            glClear(GL_COLOR_BUFFER_BIT);
            /*
             * SwapBuffers() 是一个用于交换双缓冲区的函数，常用于 OpenGL 程序中。在双缓冲机制中，应用程序可以在一个缓冲区中绘制图像，而另一个缓冲区则用于显示已经渲染
             * 好的图像。SwapBuffers() 函数用于交换这两个缓冲区，从而将已经渲染好的图像显示到屏幕上。
             */
            SwapBuffers(hdc);

            EndPaint(hwnd, &ps);
            return 0;
        }
        default: {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
}

/** Windows 桌面程序的 Main 方法 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    openLog();
    logDebug("Window Enter.");

    /* step1：注册窗口。 */
    WNDCLASSEX wndclass;
    wndclass.cbClsExtra = 0;
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.cbWndExtra = 0;
    wndclass.hbrBackground = nullptr;
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass.hIcon = nullptr;
    wndclass.hIconSm = nullptr;
    wndclass.hInstance = hInstance;
    wndclass.lpfnWndProc = GLWindowProc;//事件回调，比如关闭窗口。
    wndclass.lpszClassName = "GLWindow";
    wndclass.lpszMenuName = nullptr;
    wndclass.style = CS_VREDRAW | CS_HREDRAW;
    ATOM atom = RegisterClassEx(&wndclass);//执行注册。
    if (!atom) {
        MessageBox(nullptr, "Register Fail", "Error", MB_OK);
        return 0;
    }

    /* step2：创建窗口。 */
    HWND hwnd = CreateWindowEx(
            NULL,
            "GLWindow",//与注册的窗口明保持一致
            "OpenGL Window",
            WS_OVERLAPPEDWINDOW,
            100, 100,
            800, 600,
            nullptr,
            nullptr,
            hInstance,
            nullptr
    );

    /*
     * step3：创建 OpenGL 渲染环境。
     *
     * HDC：设备上下文句柄（Handle to Device Context），是一个指向设备上下文的句柄类型，用于在屏幕上绘制图形或执行其他与设备相关的操作。
     *
     * GetDC 函数是其中一个函数，它的作用是获取一个设备上下文（DC），也就是说，它返回一个指向设备上下文的句柄，用于在屏幕上绘制图形或执行其他与设备相关的任务。
     *
     * 例如，当您需要在窗口上绘制文本或图形时，您可以调用 GetDC 函数来获取设备上下文句柄，然后使用其他 GDI 函数（如 TextOut 或 LineTo）来在屏幕上绘制所需
     * 的内容。当您完成绘制操作后，应调用 ReleaseDC 函数释放设备上下文句柄。
     *
     * 需要注意的是，GetDC 函数获取的设备上下文句柄是一个临时资源，因此您需要在使用它时进行锁定，并在使用完毕后立即释放。如果您不释放设备上下文句柄，它就会一直
     * 占用系统资源，导致系统稳定性问题。
     */
    HDC dc = GetDC(hwnd);//获取设备，OpenGL 需要这个。

    /*
     * 设置像素格式：在创建 OpenGL 环境时设置 pixel format 是为了告诉系统我们需要使用哪种类型的像素格式来进行 OpenGL 渲染，包括颜色格式、深度缓冲区、模板缓
     * 冲区、多重采样等等。设置正确的像素格式可以提高 OpenGL 渲染的性能和质量。
     *
     * 在 Windows 平台上，可以使用 ChoosePixelFormat 和 SetPixelFormat 函数来设置像素格式。ChoosePixelFormat 函数用于从系统支持的像素格式中选择一个
     * 最符合要求的像素格式，而 SetPixelFormat 函数则用于将所选的像素格式应用于设备上下文。
     *
     * 在设置像素格式时，需要定义一个 PIXELFORMATDESCRIPTOR 结构体来描述所需的像素格式。这个结构体包括许多成员，比如颜色位数、深度位数、模板位数、多重采样等等。
     * 我们可以根据应用场景的需要来设置这些成员的值，从而得到最适合的像素格式。
     *
     * 在实际开发中，通常会根据应用的需要选择一个合适的像素格式，并将其保存下来以备后续使用。这样可以避免在每次绘制时重新选择像素格式，提高程序的运行效率。同时，需要
     * 注意不同的 OpenGL 实现可能对像素格式的要求不同，因此需要根据实际情况来选择合适的像素格式。
     */
    PIXELFORMATDESCRIPTOR pfd;//颜色描述符。
    pfd.nVersion = 1;//结构体版本号，必须设置为 1。
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);//结构体大小，以字节为单位，必须设置为 sizeof(PIXELFORMATDESCRIPTOR)。
    pfd.cColorBits = 32;//颜色位数，指定每个像素的颜色部分所占据的位数。这里设置为 32，表示每个像素使用 32 位来存储颜色信息。
    pfd.cDepthBits = 24;//深度位数，指定每个像素的深度缓冲区部分所占据的位数。这里设置为 24，表示每个像素使用 24 位来存储深度信息。
    pfd.cStencilBits = 8;//模板缓冲区位数，指定每个像素的模板缓冲区部分所占据的位数。这里设置为 8，表示每个像素使用 8 位来存储模板信息。
    pfd.iPixelType = PFD_TYPE_RGBA;//表示使用 RGBA 颜色模式。
    pfd.iLayerType = PFD_MAIN_PLANE;//图层类型，必须设置为 PFD_MAIN_PLANE。
    pfd.dwFlags = PFD_DRAW_TO_WINDOW /*画到窗口上*/ | PFD_SUPPORT_OPENGL /*颜色要支持 OpenGL*/ | PFD_DOUBLEBUFFER; /*支持双缓冲*/
    /*
     * 上面设置的像素格式使用 RGBA 颜色模式，每个像素使用 32 位（即 4 字节）存储颜色信息。因此，每个像素占用 4 字节内存。
     * 需要注意的是，该像素格式中还包括 16 位深度缓冲区，但是深度缓冲区并不是针对每个像素单独存储的，而是用于存储场景中各个物体之间的深度信息，以便进行遮挡
     * 测试等操作。因此，深度缓冲区的内存占用量不能直接算作每个像素占用的内存。
     */
    //调用函数根据设置的属性去选择颜色描述符。
    int pixelFormat = ChoosePixelFormat(dc, &pfd);
    //选择好就设置进去。
    SetPixelFormat(dc, pixelFormat, &pfd);


    /*
     * 在获取设备上下文句柄后，您需要创建 OpenGL 渲染环境，以便在 DC 上进行 OpenGL 绘制。您可以使用 Win32 AP I的 wglCreateContext
     * 和 wglMakeCurrent 函数来创建和设置 OpenGL 渲染环境，如下所示：
     *
     * HGLRC 是一个缩写，它的全称是“OpenGL 渲染环境句柄（OpenGL Rendering Context Handle）”。在 Win32 API 中，HGLRC 是一个指向 OpenGL 渲染环境的句柄
     * 类型，可以用来标识和操作 OpenGL 渲染环境。
     *
     * OpenGL 渲染环境是一个封装了 OpenGL 状态的对象，它保存了 OpenGL 的当前状态，包括渲染模式、颜色、材质、光照、纹理、深度缓冲等。当您使用 OpenGL 绘制时，必
     * 须先创建一个 OpenGL 渲染环境，并将其设置为当前上下文，才能进行绘制操作。
     *
     * 在 Win32 API 中，您可以使用 wglCreateContext 函数创建一个 OpenGL 渲染环境，并返回一个指向该环境的 HGLRC句 柄。使用 wglMakeCurrent 函数可以将创建
     * 的 OpenGL 渲染环境设置为当前上下文，以便进行 OpenGL 绘制操作。最后，使用 wglDeleteContext 函数可以删除 OpenGL渲 染环境，释放相关资源。
     *
     * 需要注意的是，每个线程只能拥有一个当前的 OpenGL 渲染环境。在多线程环境下，您需要在每个线程中创建独立的 OpenGL 渲染环境，并使用 wglMakeCurrent 函数将其
     * 设置为当前上下文，以便在每个线程中进行 OpenGL 绘制操作。
     */
    HGLRC rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);

    /*
     * step4：展示窗口。
     */
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    /*
     * step5：防止窗口退出，循环地监听事件。
     */
    MSG msg;
    // 这个 while 的执行跟刷新率有关系，比如 60hz 的刷新率每秒跑 60 次。
    while (true) {
        if (PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


    logDebug("Window Exit.");
    closeLog();

    return 0;
}