#include <Windows.h>
#include <LogKit.h>

/**
 * 窗口事件回调函数。
 *
 * @param hwnd 指定当前窗口的句柄。HWND 是一个句柄类型，用于表示 Windows 窗口的句柄，它是一个唯一标识符，用于识别窗口。
 * @param msg 指定当前消息的类型。UINT 是一个无符号整数类型，用于表示 Windows 消息的类型，例如键盘事件、鼠标事件、绘图事件等。
 * @param wParam 指定消息的附加信息。WPARAM 是一个无符号整数类型，用于表示与消息相关的附加信息，例如按下的键码、鼠标的位置等。
 * @param lParam 指定消息的附加信息。LPARAM 是一个长整数类型，用于表示与消息相关的附加信息，例如鼠标点击的时间戳、窗口的坐标等。
 * @return LRESULT CALLBACK 指定该函数的返回值类型。LRESULT 是一个长整数类型，用于表示函数的返回值，通常用于返回处理结果或错误代码。CALLBACK 是一个宏定义，用于指定该函数是一个回调函数。
 */
LRESULT CALLBACK GLWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // 画一个矩形
            RECT rect = {100, 100, 300, 300};
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0)); //创建一个红色画刷
            FillRect(hdc, &rect, hBrush); //使用画刷填充矩形
            DeleteObject(hBrush); //释放画刷资源

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

/**
 * WinMain 是 Win32 API 中用于启动 Windows 应用程序的主函数，它类似于 C 语言中的 main 函数。当应用程序启动时，Windows 将首先调用 WinMain 函数，
 * 该函数会初始化应用程序并创建主窗口。
 *
 * @param hInstance 应用程序实例句柄。该参数是一个句柄，用于标识当前应用程序的实例。在调用 WinMain 函数之前，系统会自动为当前应用程序创建一个实例，并将该实例的句柄传递给 hInstance 参数。
 * @param hPrevInstance 先前应用程序实例句柄。该参数已经被废弃，现在始终为 NULL。
 * @param lpCmdLine 命令行参数。该参数是一个字符串指针，指向包含应用程序启动时传递的命令行参数的字符串。这些参数通常用于指定应用程序启动时的一些选项或参数。
 * @param nShowCmd 窗口显示状态。该参数指定了应用程序主窗口的显示状态，例如是否最大化、最小化、隐藏等。可以使用预定义的常量，例如 SW_SHOW 表示显示窗口并将其置于最前面，SW_HIDE 表示隐藏窗口，SW_MAXIMIZE 表示最大化窗口等。
 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    openLog();
    logDebug("WinMain Enter.");

    /*
     * step1：注册窗口（窗口需要先注册才能被使用）。
     *
     * WNDCLASS 是一个结构体，用于描述窗口类的基本信息，例如窗口过程、背景色、图标等。而 WNDCLASSEX 是对 WNDCLASS 的扩展，它新增了一个字段 cbSize，
     * 用于指定结构体的大小，从而避免了不同版本的 Windows 对结构体大小的差异。
     */
    WNDCLASSEX wndclass;
    wndclass.cbClsExtra = 0;//分配给窗口类额外的字节数。可以使用这些额外的字节来存储与窗口类相关的自定义数据。
    wndclass.cbSize = sizeof(WNDCLASSEX);//结构体的大小，以字节为单位。应该设置为 sizeof(WNDCLASSEX)。
    wndclass.cbWndExtra = 0;//分配给每个窗口额外的字节数。可以使用这些额外的字节来存储与窗口实例相关的自定义数据。
    wndclass.hbrBackground = nullptr;//窗口类的背景刷句柄。背景刷用于填充窗口客户区的背景色。
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);//窗口类的光标句柄。光标通常用于鼠标指针在窗口客户区内移动时的显示。这里让鼠标处于 Loading 状态。
    wndclass.hIcon = nullptr;//窗口类的大图标句柄。大图标通常用于窗口标题栏和 Alt+Tab 切换时显示的图标。
    wndclass.hIconSm = nullptr;//窗口类的小图标句柄。小图标通常用于窗口标题栏左侧和 Alt+Tab 切换时显示的图标。
    wndclass.hInstance = hInstance;//窗口类所属的应用程序实例句柄。应用程序实例是指应用程序在内存中运行时的一个实例，每个实例都有自己的内存空间和资源。
    wndclass.lpfnWndProc = GLWindowProc;//窗口过程的地址。窗口过程是处理窗口消息的函数，它会接收并处理系统发送的各种消息，例如窗口大小变化、鼠标点击等。在创建窗口时需要指定窗口过程的地址。
    wndclass.lpszClassName = "GLWindow";//窗口类的名称。在调用 CreateWindowEx 函数时需要指定该窗口类的名称，以便创建窗口。
    wndclass.lpszMenuName = nullptr;//窗口类关联的菜单名称。如果窗口有关联的菜单，则设置该参数为菜单的名称。
    wndclass.style = CS_VREDRAW | CS_HREDRAW;//窗口类的样式。可以使用多个样式进行组合，例如 CS_HREDRAW | CS_VREDRAW 表示当窗口水平或垂直尺寸发生变化时，窗口客户区需要进行重绘。

    //ATOM 是一个无符号短整型数值，用于标识一个原子。原子是一个系统范围内的唯一标识符，用于识别字符串和其他数据。
    ATOM atom = RegisterClassEx(&wndclass);//执行注册。
    if (!atom) {
        //MessageBox 函数是一个用于显示消息框的 Win32 API 函数。
        MessageBox(nullptr, "Register Fail", "Error", MB_OK);
        return 0;
    }

    /*
     * step2：创建窗口（注册好窗口之后，就可以创建了）。
     *
     * 1. 它可以创建一个指定大小、样式和位置的窗口，并返回该窗口的句柄 HWND。
     * 2. HWND 是 Win32 API 中表示窗口句柄的数据类型，它是 "Handle to Window" 的缩写。
     */
    HWND hwnd = CreateWindowEx(
            0,//窗口扩展样式。该参数指定了窗口的扩展样式，这些样式可以用于改变窗口的外观和行为。通常情况下，该参数可以设置为 0。
            "GLWindow",//窗口类名。窗口类是指定创建窗口时使用的类别，它包含了窗口的一些默认行为和属性。在调用 CreateWindowEx 函数之前，必须先注册窗口类，可以使用 RegisterClassEx 函数进行注册。这个参数指定了使用哪个已注册的窗口类创建窗口。
            "OpenGL Window", //窗口标题。该参数用于设置窗口的标题栏文本，即窗口顶部显示的文本。
            WS_OVERLAPPEDWINDOW,//窗口样式。该参数指定了创建的窗口的风格，例如是否有标题栏、边框、最大化和最小化按钮等。可以使用多个风格组合，例如 WS_OVERLAPPEDWINDOW 就包括了窗口边框、标题栏、最大化和最小化按钮等。
            100, 100,//窗口位置。
            800, 600,//窗口初始大小。
            nullptr,//父窗口句柄。该参数指定了创建的窗口的父窗口，如果没有父窗口则设为 NULL。
            nullptr,//菜单句柄。该参数指定了创建的窗口关联的菜单句柄，如果没有菜单则设为 NULL。
            hInstance,//应用程序实例句柄。该参数指定了创建窗口所属的应用程序实例。
            nullptr//附加参数。该参数可用于传递额外的创建窗口时需要的数据。通常设为 NULL。
    );

    /*
     * step3：展示窗口。
     *
     * SW_SHOW 是一个预定义的常量，用于指定窗口的显示状态。当调用 ShowWindow 函数时，可以将 SW_SHOW 作为参数传递给该函数，以显示窗口并将其置于最前面。SW_SHOW 的值为 5，表示显示窗口并激活它。除了 SW_SHOW 之外，还有以下预定义的常量可用于指定窗口的显示状态：
     *
     *  - SW_HIDE：隐藏窗口。
     *  - SW_MAXIMIZE：最大化窗口。
     *  - SW_MINIMIZE：最小化窗口。
     *  - SW_RESTORE：还原窗口，将其恢复到之前的大小和位置。
     *  - SW_SHOWMAXIMIZED：显示窗口并将其最大化。
     *  - SW_SHOWMINIMIZED：显示窗口并将其最小化。
     *  - SW_SHOWMINNOACTIVE：显示窗口并将其最小化，但不激活窗口。
     *  - SW_SHOWNA：显示窗口，但不激活它。
     *  - SW_SHOWNOACTIVATE：显示窗口，但不激活它。
     *  - SW_SHOWNORMAL：显示窗口，并将其还原到默认大小和位置。
     */
    ShowWindow(hwnd, SW_SHOW);

    /*
     * UpdateWindow 用于更新指定窗口的客户区域，即强制立即重绘窗口的内容。该函数会发送一个 WM_PAINT 消息给指定窗口，以触发窗口客户区域的重绘。
     *
     *  1. 在 Windows 中，窗口的重绘通常是由系统自动完成的，即当窗口需要重绘时，系统会发送一个 WM_PAINT 消息给窗口，然后窗口处理该消息，并调用 BeginPaint 函数获取绘图设备上下文 HDC，接着使用 GDI 函数进行绘图操作，最后使用 EndPaint 函数释放绘图设备上下文。这种方式可以确保窗口的重绘是在正确的时间和正确的方式下完成的。
     *  2. 然而，在某些情况下，窗口的重绘可能需要立即完成，例如当窗口的内容发生变化时需要立即更新窗口的显示。这时就可以使用 UpdateWindow 函数来强制立即重绘窗口的内容，以确保窗口的显示是及时的。
     *  3. 需要注意的是，过度地使用 UpdateWindow 可能会导致程序的性能问题，因为它会强制触发窗口的重绘，而重绘操作通常是比较耗费资源的。因此，在使用 UpdateWindow 函数时，应该谨慎使用，并确保只在必要时使用。
     *
     *  其实这里不调用也可以。
     */
    UpdateWindow(hwnd);

    /*
     * step4：防止窗口退出，开始监听事件。
     */
    MSG msg;//用于接收消息。
    while (true) {
        /*
         * 使用 PeekMessage 函数来获取消息队列中的消息。该函数用于获取消息队列中的第一个消息，并将其存储在指定的 MSG 结构体中。如果消息队列中没有消息，则该函数会立即返回 false，表示没有消息。
         *
         *  - lpMsg：指向 MSG 结构体的指针，用于存储获取到的消息。
         *  - hwnd：指定窗口句柄，用于筛选消息。如果为 nullptr，则表示获取所有窗口的消息。
         *  - wMsgFilterMin：指定要获取的消息范围的最小值。传递 0 表示获取所有消息。
         *  - wMsgFilterMax：指定要获取的消息范围的最大值。传递 0 表示获取所有消息。
         *  - wRemoveMsg：指定如何处理获取到的消息。如果该参数为 PM_REMOVE，则表示从消息队列中移除该消息，否则不移除该消息。
         */
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            // 使用 TranslateMessage 函数将消息进行翻译。该函数用于将一些键盘和鼠标事件转化为字符消息，以便于应用程序处理。
            TranslateMessage(&msg);
            // 使用 DispatchMessage 函数将消息分发给指定的窗口过程进行处理。该函数会将消息发送给窗口过程（即上面设置的 GLWindowProc），并等待窗口过程处理完该消息后返回。
            DispatchMessage(&msg);
        }
    }

    OutputDebugString("Window Exit.");

    // 关闭控制台窗口
    logDebug("WinMain Exit.");
    closeLog();

    return 0;
}