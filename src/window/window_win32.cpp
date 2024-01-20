#include <window/window_win32.hpp>
#include <core/event/event.hpp>
#include <core/log.hpp>
#include <core/std/basic_types.hpp>
#include <cstring>
#include <windowsx.h>

namespace window
{
    namespace _internal
    {
        Win32PlatformData pd{nullptr};

        bool initPlatform()
        {
            if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
                logWarn("Failed to set process dpi awareness context");

            pd.instance = GetModuleHandleW(nullptr);
            pd.win32class = {sizeof(pd.win32class)};
            pd.win32class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            pd.win32class.lpfnWndProc = wndProc;
            pd.win32class.hInstance = pd.instance;
            pd.win32class.lpszClassName = L"APP3DWINDOW";
            pd.win32class.hCursor = LoadCursor(NULL, IDC_ARROW);
            pd.win32class.hIcon = LoadIconW(pd.instance, L"WINDOW_ICON");
            if (!pd.win32class.hIcon)
                pd.win32class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            ATOM classAtom = RegisterClassExW(&pd.win32class);
            return classAtom != 0;
        }

        static bool isMaximized(HWND hwnd)
        {
            WINDOWPLACEMENT placement = {0};
            placement.length = sizeof(WINDOWPLACEMENT);
            if (GetWindowPlacement(hwnd, &placement))
                return placement.showCmd == SW_SHOWMAXIMIZED;
            return false;
        }

        DWORD getWindowStyle(CreationFlags flags)
        {
            DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            if (flags & CreationFlagsBits::fullscreen)
                style |= WS_POPUP;
            if (flags & CreationFlagsBits::snapped)
                style |= WS_SYSMENU;
            if (flags & CreationFlagsBits::minimizebox)
                style |= WS_MINIMIZEBOX;
            if (flags & CreationFlagsBits::maximizebox)
                style |= WS_MAXIMIZEBOX;
            if (flags & CreationFlagsBits::resizable)
                style |= WS_THICKFRAME;
            if (flags & CreationFlagsBits::decorated)
                style |= WS_CAPTION;
            return style;
        }

        static io::KeyMode getKeyMods()
        {
            io::KeyMode mods;
            if (GetKeyState(VK_SHIFT) & 0x8000)
                mods |= io::KeyModeBits::shift;
            if (GetKeyState(VK_CONTROL) & 0x8000)
                mods |= io::KeyModeBits::control;
            if (GetKeyState(VK_MENU) & 0x8000)
                mods |= io::KeyModeBits::alt;
            if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
                mods |= io::KeyModeBits::super;
            if (GetKeyState(VK_CAPITAL) & 1)
                mods |= io::KeyModeBits::capsLock;
            if (GetKeyState(VK_NUMLOCK) & 1)
                mods |= io::KeyModeBits::numLock;
            return mods;
        }

        LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            Win32Window *window = (Win32Window *)GetPropW(hwnd, L"APP3DWINDOW");
            switch (uMsg)
            {
                // Handling this event allows us to extend client (paintable) area into the title bar region
                // The information is partially coming from:
                // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
                // Most important paragraph is:
                //   To remove the standard window frame, you must handle the WM_NCCALCSIZE message,
                //   specifically when its wParam value is TRUE and the return value is 0.
                //   By doing so, your application uses the entire window region as the client area,
                //   removing the standard frame.
                case WM_NCCALCSIZE:
                {
                    if (!wParam)
                        break;
                    window = reinterpret_cast<Win32Window *>(GetPropW(hwnd, L"APP3DWINDOW"));
                    if (!window || window->decorated() || window->fullscreen())
                        break;
                    pd.dpi = GetDpiForWindow(hwnd);
                    pd.frameX = GetSystemMetricsForDpi(SM_CXFRAME, pd.dpi);
                    pd.frameY = GetSystemMetricsForDpi(SM_CYFRAME, pd.dpi);
                    pd.padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, pd.dpi);

                    NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)lParam;
                    RECT *clentRect = params->rgrc;
                    clentRect->right -= pd.frameX + pd.padding;
                    clentRect->left += pd.frameX + pd.padding;
                    clentRect->bottom -= pd.frameY + pd.padding;

                    if (isMaximized(hwnd))
                        clentRect->top += pd.padding;
                    return 0;
                }
                case WM_CREATE:
                {
                    CREATESTRUCT *createStruct = reinterpret_cast<CREATESTRUCT *>(lParam);
                    window = reinterpret_cast<Win32Window *>(createStruct->lpCreateParams);
                    if (!window)
                        break;
                    SetPropW(hwnd, L"APP3DWINDOW", reinterpret_cast<HANDLE>(window));
                    MonitorInfo monitorInfo = getPrimaryMonitorInfo();
                    pd.screenWidth = monitorInfo.width;
                    pd.screenHeight = monitorInfo.height;
                    if (window->fullscreen())
                    {
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, pd.screenWidth, pd.screenHeight, SWP_SHOWWINDOW);
                        return 0;
                    }

                    if (window->decorated())
                        break;
                    RECT sizeRect;
                    GetWindowRect(hwnd, &sizeRect);

                    // Inform the application of the frame change to force redrawing with the new
                    // client area that is extended into the title bar
                    SetWindowPos(hwnd, NULL, sizeRect.left, sizeRect.top, sizeRect.right - sizeRect.left,
                                 sizeRect.bottom - sizeRect.top, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
                    break;
                }
                case WM_NCCREATE:
                    EnableNonClientDpiScaling(hwnd);
                    break;
                case WM_NCHITTEST:
                {
                    if (!gWindowEvents.NCHitTest)
                        break;
                    LRESULT hit = DefWindowProcW(hwnd, uMsg, wParam, lParam);
                    switch (hit)
                    {
                        case HTNOWHERE:
                        case HTRIGHT:
                        case HTLEFT:
                        case HTTOPLEFT:
                        case HTTOP:
                        case HTTOPRIGHT:
                        case HTBOTTOMRIGHT:
                        case HTBOTTOM:
                        case HTBOTTOMLEFT:
                            return hit;
                    }
                    POINT cursorPoint = {0};
                    cursorPoint.x = LOWORD(lParam);
                    cursorPoint.y = HIWORD(lParam);
                    ScreenToClient(hwnd, &cursorPoint);
                    if (cursorPoint.y > 0 && cursorPoint.y < pd.frameY + pd.padding)
                        return HTTOP;
                    Win32NativeEvent event("window:NCHitTest", window, uMsg, wParam, lParam, &hit);
                    gWindowEvents.NCHitTest->invoke(event);
                    return hit;
                }
                case WM_NCLBUTTONDOWN:
                {
                    if (gWindowEvents.NCLMouseClick)
                    {
                        LRESULT res{-1};
                        Win32NativeEvent event("window:NCLMouseClick", window, uMsg, wParam, lParam, &res);
                        gWindowEvents.NCLMouseClick->invoke(event);
                        if (res != -1)
                            return res;
                    }
                    break;
                }
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_XBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP:
                {
                    io::MouseKey button;
                    io::KeyPressState action;
                    switch (uMsg)
                    {
                        case WM_LBUTTONDOWN:
                            button = io::MouseKey::left;
                            action = io::KeyPressState::press;
                            break;
                        case WM_LBUTTONUP:
                            button = io::MouseKey::left;
                            action = io::KeyPressState::release;
                            break;
                        case WM_RBUTTONDOWN:
                            button = io::MouseKey::right;
                            action = io::KeyPressState::press;
                            break;
                        case WM_RBUTTONUP:
                            button = io::MouseKey::right;
                            action = io::KeyPressState::release;
                            break;
                        case WM_MBUTTONDOWN:
                            button = io::MouseKey::middle;
                            action = io::KeyPressState::press;
                            break;
                        case WM_MBUTTONUP:
                            button = io::MouseKey::middle;
                            action = io::KeyPressState::release;
                            break;
                        default:
                            button = io::MouseKey::unknown;
                            action = io::KeyPressState::release;
                            break;
                    };
                    emitWindowEvent(gWindowEvents.mouseClickEvents, "window:mouseClick", window, button, action);
                    break;
                }
                case WM_SETFOCUS:
                {
                    window->_focused = true;
                    emitWindowEvent(gWindowEvents.focusEvents, "window:focus", window, true);
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_INPUTSINK, hwnd};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        logError("Failed to register RAWINPUTDEVICE");
                    else
                        window->_rawInput = true;
                    break;
                }
                case WM_KILLFOCUS:
                {
                    window->_focused = false;
                    emitWindowEvent(gWindowEvents.focusEvents, "window:focus", window, false);
                    if (!window->_rawInput)
                        break;
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_REMOVE, NULL};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        logError("Failed to remove raw input device");
                    else
                        window->_rawInput = false;
                    break;
                }
                case WM_CHAR:
                case WM_SYSCHAR:
                {
                    if (wParam >= 0xd800 && wParam <= 0xdbff)
                        window->_highSurrogate = (WCHAR)wParam;
                    else
                    {
                        u32 codepoint = 0;
                        if (wParam >= 0xdc00 && wParam <= 0xdfff)
                        {
                            if (window->_highSurrogate)
                            {
                                codepoint += (window->_highSurrogate - 0xd800) << 10;
                                codepoint += (WCHAR)wParam - 0xdc00;
                                codepoint += 0x10000;
                            }
                        }
                        else
                            codepoint = (WCHAR)wParam;
                        window->_highSurrogate = 0;
                        emitWindowEvent(gWindowEvents.charInputEvents, "window:charInput", window, codepoint);
                    }
                    if (uMsg == WM_SYSCHAR)
                        break;
                    return 0;
                }
                case WM_UNICHAR:
                {
                    if (wParam == UNICODE_NOCHAR)
                    {
                        // WM_UNICHAR is not sent by Windows, but is sent by some
                        // third-party input method engine
                        // Returning TRUE here announces support for this message
                        return TRUE;
                    }
                    emitWindowEvent(gWindowEvents.charInputEvents, "window:charInput", window, wParam);
                    return 0;
                }
                case WM_SYSCOMMAND:
                {
                    switch (wParam & 0xfff0)
                    {
                        case SC_SCREENSAVE:
                        case SC_MONITORPOWER:
                        {
                            if (window->fullscreen())
                                return 0;
                            else
                                break;
                        }
                    }
                    break;
                }
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYUP:
                {

                    const auto action =
                        (HIWORD(lParam) & KF_UP) ? io::KeyPressState::release : io::KeyPressState::press;
                    const auto mods = getKeyMods();
                    io::Key key;
                    auto it = pd.keymap.find(wParam);
                    key = it != pd.keymap.end() ? it->second : io::Key::kUnknown;
                    // The Ctrl keys require special handling
                    if (wParam == VK_CONTROL)
                    {
                        if (HIWORD(lParam) & KF_EXTENDED)
                            key = io::Key::kRightControl;
                        else
                        {
                            // NOTE: Alt Gr sends Left Ctrl followed by Right Alt
                            // HACK: We only want one event for Alt Gr, so if we detect
                            //       this sequence we discard this Left Ctrl message now
                            //       and later report Right Alt normally
                            MSG next;
                            const DWORD time = GetMessageTime();

                            if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
                            {
                                if (next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN ||
                                    next.message == WM_KEYUP || next.message == WM_SYSKEYUP)
                                {
                                    if (next.wParam == VK_MENU && (HIWORD(next.lParam) & KF_EXTENDED) &&
                                        next.time == time)
                                        break;
                                }
                            }

                            // This is a regular Left Ctrl message
                            key = io::Key::kLeftControl;
                        }
                    }
                    else if (wParam == VK_PROCESSKEY)
                    {
                        // IME notifies that keys have been filtered by setting the
                        // virtual key-code to VK_PROCESSKEY
                        break;
                    }

                    if (action == io::KeyPressState::release && wParam == VK_SHIFT)
                    {
                        // HACK: Release both Shift keys on Shift up event, as when both
                        //       are pressed the first release does not emit any event
                        // NOTE: The other half of this is in _glfwPollEventsWin32
                        window->inputKey(io::Key::kLeftShift, action, mods);
                        window->inputKey(io::Key::kRightShift, action, mods);
                    }
                    else if (wParam == VK_SNAPSHOT)
                    {
                        // HACK: Key down is not reported for the Print Screen key
                        window->inputKey(key, io::KeyPressState::press, mods);
                        window->inputKey(key, io::KeyPressState::release, mods);
                    }
                    else
                        window->inputKey(key, action, mods);
                    // Prevent Alt to Menu call Behavior
                    if (wParam == VK_MENU)
                        return 0;
                    break;
                }
                case WM_MOUSEMOVE:
                {
                    Point2D pos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    if (!window->_cursorTracked)
                    {
                        TRACKMOUSEEVENT tme;
                        ZeroMemory(&tme, sizeof(tme));
                        tme.cbSize = sizeof(tme);
                        tme.dwFlags = TME_LEAVE;
                        tme.hwndTrack = window->_hwnd;
                        TrackMouseEvent(&tme);
                        window->_cursorTracked = true;
                        emitWindowEvent(gWindowEvents.cursorEnterEvents, "window:cursorEnter", window, true);
                    }
                    return 0;
                }
                case WM_MOUSELEAVE:
                    window->_cursorTracked = false;
                    emitWindowEvent(gWindowEvents.cursorEnterEvents, "window:cursorEnter", window, false);
                    return 0;
                case WM_MOUSEWHEEL:
                    emitWindowEvent(gWindowEvents.scrollEvents, "window:scroll", window,
                                    (SHORT)HIWORD(wParam) / (double)WHEEL_DELTA);
                    return 0;
                case WM_MOUSEHWHEEL:
                {
                    // This message is only sent on Windows Vista and later
                    // NOTE: The X-axis is inverted for consistency with macOS and X11
                    emitWindowEvent(gWindowEvents.scrollEvents, "window:scroll", window,
                                    -((SHORT)HIWORD(wParam) / (double)WHEEL_DELTA));
                    return 0;
                }
                case WM_SIZE:
                {
                    Point2D dimenstions(LOWORD(lParam), HIWORD(lParam));
                    if (window->_minimized != (wParam == SIZE_MINIMIZED))
                        emitWindowEvent(gWindowEvents.minimizeEvents, "window:minimize", window, dimenstions);
                    if (window->_maximized != (wParam == SIZE_MAXIMIZED))
                        emitWindowEvent(gWindowEvents.maximizeEvents, "window:maximize", window, dimenstions);
                    if (dimenstions != window->_dimenstions)
                    {
                        window->_dimenstions = dimenstions;
                        emitWindowEvent(gWindowEvents.resizeEvents, "window:resize", window, dimenstions);
                    }
                    return 0;
                }
                case WM_MOVE:
                    emitWindowEvent(gWindowEvents.moveEvents, "window:move", window,
                                    Point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    break;
                case WM_GETMINMAXINFO:
                {
                    if (!window)
                        break;
                    MINMAXINFO *mmi = (MINMAXINFO *)lParam;
                    mmi->ptMinTrackSize.x = window->_resizeLimit.x;
                    mmi->ptMinTrackSize.y = window->_resizeLimit.y;
                    return 0;
                }
                case WM_ERASEBKGND:
                    return TRUE;
                case WM_DPICHANGED:
                {
                    const float xscale = HIWORD(wParam) / 96.0f;
                    const float yscale = LOWORD(wParam) / 96.0f;
                    emitWindowEvent(gWindowEvents.dpiChangedEvents, "window:dpiChanged", window, xscale, yscale);
                    break;
                }
                case WM_SETCURSOR:
                {
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                    break;
                }
                case WM_INPUT:
                {
                    if (!window->_rawInput)
                        break;
                    UINT dwSize;
                    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                    if (dwSize > window->_rawInputSize)
                    {
                        if (window->_rawInputData)
                            free(window->_rawInputData);
                        window->_rawInputData = (LPBYTE)malloc(dwSize);
                        window->_rawInputSize = dwSize;
                    }
                    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, window->_rawInputData, &dwSize,
                                        sizeof(RAWINPUTHEADER)) != dwSize)
                    {
                        logError("GetRawInputData does not return correct size");
                        break;
                    }
                    RAWINPUT *raw = (RAWINPUT *)window->_rawInputData;

                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        i32 dx = raw->data.mouse.lLastX;
                        i32 dy = raw->data.mouse.lLastY;
                        emitWindowEvent(gWindowEvents.cursorPosEvents, "window:cursorPos", window, Point2D{dx, dy});
                    }
                    return 0;
                }
                case WM_CLOSE:
                    window->_readyToClose = true;
                    break;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
                default:
                    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
            }
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }

        void destroyPlatform()
        {
            if (pd.instance)
            {
                if (pd.win32class.hCursor)
                    DestroyCursor(pd.win32class.hCursor);
                if (pd.win32class.hIcon)
                    DestroyIcon(pd.win32class.hIcon);
                UnregisterClassW(pd.win32class.lpszClassName, pd.instance);
            }
        }
    } // namespace _internal

    MonitorInfo getPrimaryMonitorInfo()
    {
        HMONITOR hMonitor = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if (GetMonitorInfo(hMonitor, &monitorInfo))
        {
            return {.width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                    .height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top};
        }
        else
            return {};
    }

    Win32Window::Win32Window(const std::string &title, int width, int height, CreationFlags flags)
        : WindowBase(title, width == -1 ? CW_USEDEFAULT : width, height == -1 ? CW_USEDEFAULT : height, flags),
          _title(convertUTF8toUTF16(title)),
          _style(_internal::getWindowStyle(flags)),
          _exStyle(WS_EX_APPWINDOW)
    {

        _hwnd = CreateWindowExW(_exStyle, _internal::pd.win32class.lpszClassName, (LPCWSTR)_title.c_str(), _style,
                                CW_USEDEFAULT, CW_USEDEFAULT, _dimenstions.x, _dimenstions.y, nullptr, nullptr,
                                _internal::pd.instance, (LPVOID)this);
        if (!_hwnd)
            throw std::runtime_error("Failed to create window");
        if (!(flags & CreationFlagsBits::hidden))
            ShowWindow(_hwnd, SW_SHOWNORMAL);
    }

    Win32Window::~Win32Window()
    {
        if (_hwnd)
        {
            RemovePropW(_hwnd, L"APP3DWINDOW");
            DestroyWindow(_hwnd);
            _hwnd = nullptr;
        }

        if (_rawInputData)
        {
            free(_rawInputData);
            _rawInputData = nullptr;
            _rawInputSize = 0;
        }
    }

    void Win32Window::showWindow()
    {
        if (!hidden())
            return;
        ShowWindow(_hwnd, SW_SHOWNORMAL);
        _flags &= ~CreationFlagsBits::hidden;
    }

    void Win32Window::hideWindow()
    {
        if (hidden())
            return;
        ShowWindow(_hwnd, SW_HIDE);
        _flags |= CreationFlagsBits::hidden;
    }

    void Win32Window::title(const std::string &title)
    {
        _title = convertUTF8toUTF16(title);
        SetWindowTextW(_hwnd, (LPCWSTR)_title.c_str());
    }

    void Win32Window::enableFullscreen()
    {
        _flags |= CreationFlagsBits::fullscreen;
        SetWindowLongPtr(_hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
        SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, _internal::pd.screenWidth, _internal::pd.screenHeight, SWP_SHOWWINDOW);
    }

    void Win32Window::disableFullscreen()
    {
        _flags &= ~CreationFlagsBits::fullscreen;
        SetWindowLongPtr(_hwnd, GWL_STYLE, _style);
        SetWindowPos(_hwnd, HWND_NOTOPMOST, 0, 0, _dimenstions.x, _dimenstions.y, SWP_SHOWWINDOW);
    }

    Point2D Win32Window::cursorPosition() const
    {
        POINT pos;
        if (GetCursorPos(&pos))
        {
            ScreenToClient(_hwnd, &pos);
            return {pos.x, pos.y};
        }
        return {};
    }

    void Win32Window::cursorPosition(Point2D position)
    {
        POINT pos = {position.x, position.y};
        ClientToScreen(_hwnd, &pos);
        SetCursorPos(pos.x, pos.y);
    }

    void Win32Window::showCursor()
    {
        cursorPosition(_savedCursorPos);
        ReleaseCapture();
        ShowCursor(TRUE);
        _isCursorHidden = false;
    }

    void Win32Window::hideCursor()
    {
        _savedCursorPos = cursorPosition();
        SetCapture(_hwnd);
        ShowCursor(FALSE);
        ClipCursor(NULL);
        _isCursorHidden = true;
    }

    void waitEvents()
    {
        WaitMessage();
        pollEvents();
    }

    void pollEvents()
    {
        MSG msg = {};
        HWND hwnd = GetActiveWindow();

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Win32Window *window = (Win32Window *)GetPropW(hwnd, L"APP3DWINDOW");
        if (!window)
            return;

        if (window->_isCursorHidden && window->cursorPosition() != window->_savedCursorPos)
        {
            Point2D pos = window->_dimenstions / 2.0f;
            window->cursorPosition(pos);
        }
    }

    void waitEventsTimeout(f64 timeout)
    {
        MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD)(timeout * 1e3), QS_ALLINPUT);
        pollEvents();
    }
} // namespace window