#include <core/log.hpp>
#include <window/platform_win32.hpp>
#include <window/window.hpp>
#include <windowsx.h>

namespace window
{
    namespace platform
    {
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
            WindowPlatformData *window = (WindowPlatformData *)GetPropW(hwnd, L"APP3DWINDOW");
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
                    window = reinterpret_cast<WindowPlatformData *>(GetPropW(hwnd, L"APP3DWINDOW"));
                    env.context->dpi = GetDpiForWindow(hwnd);
                    env.context->frameX = GetSystemMetricsForDpi(SM_CXFRAME, env.context->dpi);
                    env.context->frameY = GetSystemMetricsForDpi(SM_CYFRAME, env.context->dpi);
                    env.context->padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, env.context->dpi);

                    if (!wParam || !window || (window->flags & CreationFlagsBits::decorated) ||
                        (window->flags & CreationFlagsBits::fullscreen))
                        break;

                    NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)lParam;
                    RECT *clentRect = params->rgrc;
                    clentRect->right -= env.context->frameX + env.context->padding;
                    clentRect->left += env.context->frameX + env.context->padding;
                    clentRect->bottom -= env.context->frameY + env.context->padding;

                    if (isMaximized(hwnd))
                        clentRect->top += env.context->padding;
                    return 0;
                }
                case WM_CREATE:
                {
                    CREATESTRUCT *createStruct = reinterpret_cast<CREATESTRUCT *>(lParam);
                    window = reinterpret_cast<WindowPlatformData *>(createStruct->lpCreateParams);
                    if (!window)
                        break;
                    SetPropW(hwnd, L"APP3DWINDOW", reinterpret_cast<HANDLE>(window));
                    MonitorInfo monitorInfo = getPrimaryMonitorInfo();
                    env.context->screenWidth = monitorInfo.width;
                    env.context->screenHeight = monitorInfo.height;
                    if (window->flags & CreationFlagsBits::fullscreen)
                    {
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, env.context->screenWidth, env.context->screenHeight,
                                     SWP_SHOWWINDOW);
                        return 0;
                    }

                    if (window->flags & CreationFlagsBits::decorated)
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
                    if (window->flags & CreationFlagsBits::decorated)
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
                    if (cursorPoint.y > 0 && cursorPoint.y < env.context->frameY + env.context->padding)
                        return HTTOP;
                    if (!eventRegistry.NCHitTest)
                        break;
                    Win32NativeEvent event("window:NCHitTest", window->owner, hwnd, uMsg, wParam, lParam, &hit);
                    eventRegistry.NCHitTest->invoke(event);
                    return hit;
                }
                case WM_NCLBUTTONDOWN:
                {
                    if (eventRegistry.NCLMouseDown)
                    {
                        LRESULT res{-1};
                        Win32NativeEvent event("window:NCLMouseDown", window->owner, hwnd, uMsg, wParam, lParam, &res);
                        eventRegistry.NCLMouseDown->invoke(event);
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
                    emitWindowEvent(eventRegistry.mouseClickEvents, "window:input:mouse", window->owner, button,
                                    action);
                    break;
                }
                case WM_SETFOCUS:
                {
                    window->focused = true;
                    emitWindowEvent(eventRegistry.focusEvents, "window:focus", window->owner, true);
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_INPUTSINK, hwnd};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        logError("Failed to register RAWINPUTDEVICE");
                    else
                        window->rawInput = true;
                    break;
                }
                case WM_KILLFOCUS:
                {
                    if (!window)
                        break;
                    window->focused = false;
                    emitWindowEvent(eventRegistry.focusEvents, "window:focus", window->owner, false);
                    if (!window->rawInput)
                        break;
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_REMOVE, NULL};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        logError("Failed to remove raw input device");
                    else
                        window->rawInput = false;
                    break;
                }
                case WM_CHAR:
                case WM_SYSCHAR:
                {
                    if (IS_HIGH_SURROGATE(wParam))
                        window->highSurrogate = wParam;
                    else if (IS_LOW_SURROGATE(wParam))
                    {
                        if (window->highSurrogate)
                        {
                            u32 codepoint = (((window->highSurrogate - 0xD800) << 10) | (wParam - 0xDC00)) + 0x10000;
                            window->highSurrogate = 0;
                            emitWindowEvent(eventRegistry.charInputEvents, "window:input:char", window->owner,
                                            codepoint);
                        }
                    }
                    else
                        emitWindowEvent(eventRegistry.charInputEvents, "window:input:char", window->owner, wParam);

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
                    emitWindowEvent(eventRegistry.charInputEvents, "window:input:char", window->owner, wParam);
                    return 0;
                }
                case WM_SYSCOMMAND:
                {
                    switch (wParam & 0xfff0)
                    {
                        case SC_SCREENSAVE:
                        case SC_MONITORPOWER:
                        {
                            if (window->flags & CreationFlagsBits::fullscreen)
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
                    io::Key key = io::Key::kUnknown;
                    switch (wParam)
                    {
                        case VK_MENU:
                            key = (HIWORD(lParam) & KF_EXTENDED) ? io::Key::kRightAlt : io::Key::kLeftAlt;
                            break;
                        case VK_SHIFT:
                        {
                            if (action == io::KeyPressState::release)
                            {
                                // HACK: Release both Shift keys on Shift up event, as when both
                                //       are pressed the first release does not emit any event
                                inputKey(window, io::Key::kLeftShift, action, mods);
                                inputKey(window, io::Key::kRightShift, action, mods);
                            }
                            else
                                key = (HIWORD(lParam) & KF_EXTENDED) ? io::Key::kRightShift : io::Key::kLeftShift;
                            break;
                        }
                        case VK_CONTROL:
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
                            break;
                        }
                        case VK_PROCESSKEY:
                            // IME notifies that keys have been filtered by setting the
                            // virtual key-code to VK_PROCESSKEY
                            break;
                        case VK_SNAPSHOT:
                            // HACK: Key down is not reported for the Print Screen key
                            inputKey(window, key, io::KeyPressState::press, mods);
                            inputKey(window, key, io::KeyPressState::release, mods);
                            break;
                        default:
                        {
                            auto it = env.context->keymap.find(wParam);
                            key = it != env.context->keymap.end() ? it->second : io::Key::kUnknown;
                            break;
                        }
                    }

                    if (key != io::Key::kUnknown)
                        inputKey(window, key, action, mods);
                    // Prevent Alt to call Menu Behavior
                    if (wParam == VK_MENU)
                        return 0;
                    break;
                }
                case WM_MOUSEMOVE:
                {
                    if (!window->cursorTracked)
                    {
                        TRACKMOUSEEVENT tme;
                        ZeroMemory(&tme, sizeof(tme));
                        tme.cbSize = sizeof(tme);
                        tme.dwFlags = TME_LEAVE;
                        tme.hwndTrack = window->hwnd;
                        TrackMouseEvent(&tme);
                        window->cursorTracked = true;
                        emitWindowEvent(eventRegistry.cursorEnterEvents, "window:cursor:enter", window->owner, true);
                    }
                    emitWindowEvent(eventRegistry.cursorPosAbsEvents, "window:cursorPosAbs", window->owner,
                                    Point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    return 0;
                }
                case WM_MOUSELEAVE:
                    window->cursorTracked = false;
                    emitWindowEvent(eventRegistry.cursorEnterEvents, "window:cursor:enter", window->owner, false);
                    return 0;
                case WM_MOUSEWHEEL:
                    emitWindowEvent(eventRegistry.scrollEvents, "window:scroll", window->owner, 0,
                                    (SHORT)HIWORD(wParam) / (double)WHEEL_DELTA);
                    return 0;
                case WM_MOUSEHWHEEL:
                {
                    logInfo("hscroll");
                    // This message is only sent on Windows Vista and later
                    // NOTE: The X-axis is inverted for consistency with macOS and X11
                    emitWindowEvent(eventRegistry.scrollEvents, "window:scroll", window->owner,
                                    -((SHORT)HIWORD(wParam) / (double)WHEEL_DELTA), 0);
                    return 0;
                }
                case WM_SIZE:
                {
                    Point2D dimenstions(LOWORD(lParam), HIWORD(lParam));
                    if ((window->flags & CreationFlagsBits::minimized) != (wParam == SIZE_MINIMIZED))
                    {
                        window->flags ^= CreationFlagsBits::minimized;
                        emitWindowEvent(eventRegistry.minimizeEvents, "window:minimize", window->owner,
                                        window->flags & CreationFlagsBits::minimized);
                    }
                    if ((window->flags & CreationFlagsBits::maximized) != (wParam == SIZE_MAXIMIZED))
                    {
                        window->flags ^= CreationFlagsBits::maximized;
                        emitWindowEvent(eventRegistry.maximizeEvents, "window:maximize", window->owner,
                                        window->flags & CreationFlagsBits::maximized);
                    }
                    if (dimenstions != window->dimenstions)
                    {
                        window->dimenstions = dimenstions;
                        emitWindowEvent(eventRegistry.resizeEvents, "window:resize", window->owner, dimenstions);
                    }
                    return 0;
                }
                case WM_MOVE:
                    emitWindowEvent(eventRegistry.moveEvents, "window:move", window->owner,
                                    Point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    break;
                case WM_GETMINMAXINFO:
                {
                    if (!window)
                        break;
                    MINMAXINFO *mmi = (MINMAXINFO *)lParam;
                    mmi->ptMinTrackSize.x = window->resizeLimit.x;
                    mmi->ptMinTrackSize.y = window->resizeLimit.y;
                    return 0;
                }
                case WM_ERASEBKGND:
                    return TRUE;
                case WM_DPICHANGED:
                {
                    const float xscale = HIWORD(wParam) / 96.0f;
                    const float yscale = LOWORD(wParam) / 96.0f;
                    emitWindowEvent(eventRegistry.dpiChangedEvents, "window:dpiChanged", window->owner, xscale, yscale);
                    break;
                }
                case WM_SETCURSOR:
                {
                    if (LOWORD(lParam) == HTCLIENT)
                    {
                        if (window->cursor->valid())
                            window->cursor->assign();
                        else
                            SetCursor(LoadCursor(NULL, IDC_ARROW));
                        return TRUE;
                    }
                    break;
                }
                case WM_INPUT:
                {
                    if (!window->rawInput)
                        break;
                    UINT dwSize;
                    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                    if (dwSize > window->rawInputSize)
                    {
                        if (window->rawInputData)
                            free(window->rawInputData);
                        window->rawInputData = (LPBYTE)malloc(dwSize);
                        window->rawInputSize = dwSize;
                    }
                    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, window->rawInputData, &dwSize,
                                        sizeof(RAWINPUTHEADER)) != dwSize)
                    {
                        logError("GetRawInputData does not return correct size");
                        break;
                    }
                    RAWINPUT *raw = (RAWINPUT *)window->rawInputData;

                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        i32 dx = raw->data.mouse.lLastX;
                        i32 dy = raw->data.mouse.lLastY;
                        emitWindowEvent(eventRegistry.cursorPosEvents, "window:cursor:move", window->owner,
                                        Point2D{dx, dy});
                    }
                    return 0;
                }
                case WM_CLOSE:
                    window->readyToClose = true;
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
            if (env.context)
            {
                if (env.context->instance)
                {
                    if (env.context->win32class.hIcon)
                        DestroyIcon(env.context->win32class.hIcon);
                    UnregisterClassW(env.context->win32class.lpszClassName, env.context->instance);
                }
                delete env.context;
            }
        }

        void initTimer() { QueryPerformanceFrequency((LARGE_INTEGER *)&env.timer.frequency); }

        u64 getTimeValue()
        {
            u64 value;
            QueryPerformanceCounter((LARGE_INTEGER *)&value);
            return value;
        }

        u64 getTimeFrequency() { return env.timer.frequency; }

        bool initPlatform()
        {
            if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
                logWarn("Failed to set process dpi awareness context");
            PlatformContext *context = new PlatformContext;
            context->instance = GetModuleHandleW(nullptr);
            context->win32class = {sizeof(context->win32class)};
            context->win32class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            context->win32class.lpfnWndProc = wndProc;
            context->win32class.hInstance = context->instance;
            context->win32class.lpszClassName = L"APP3DWINDOWLIB";
            context->win32class.hCursor = LoadCursor(NULL, IDC_ARROW);
            context->win32class.hIcon = LoadIconW(context->instance, L"WINDOW_ICON");
            if (!context->win32class.hIcon)
            {
                logWarn("Failed to load window icon");
                context->win32class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            }
            if (RegisterClassExW(&context->win32class))
            {
                env.context = context;
                return true;
            }
            else
            {
                delete context;
                return false;
            }
        }

        HINSTANCE AccessBridge::instance() const { return env.context->instance; }

        HWND AccessBridge::hwnd() const { return _impl->hwnd; }
    } // namespace platform

    MonitorInfo getPrimaryMonitorInfo()
    {
        HMONITOR hMonitor = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if (GetMonitorInfoW(hMonitor, &monitorInfo))
        {
            return {.xpos = monitorInfo.rcWork.right,
                    .ypos = monitorInfo.rcWork.bottom,
                    .width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                    .height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top};
        }
        else
            return {};
    }

    Window::Window(const std::string &title, i32 width, i32 height, CreationFlags flags)
        : _platform(nullptr), _accessBridge(nullptr)
    {
        _platform = new platform::WindowPlatformData;
        _platform->owner = this;
        _platform->title = convertUTF8toUTF16(title);
        _platform->dimenstions = {width == -1 ? CW_USEDEFAULT : width, height == -1 ? CW_USEDEFAULT : height};
        _platform->flags = flags;
        _platform->style = platform::getWindowStyle(flags);
        _platform->exStyle = WS_EX_APPWINDOW;
        _platform->hwnd = nullptr;
        _platform->cursor = Cursor::defaultCursor();
        _accessBridge = new platform::AccessBridge(_platform);
        _platform->hwnd = CreateWindowExW(_platform->exStyle, platform::env.context->win32class.lpszClassName,
                                          (LPCWSTR)_platform->title.c_str(), _platform->style, CW_USEDEFAULT,
                                          CW_USEDEFAULT, _platform->dimenstions.x, _platform->dimenstions.y, nullptr,
                                          nullptr, platform::env.context->instance, (LPVOID)_platform);
        if (!_platform->hwnd)
            throw std::runtime_error("Failed to create window");
        if (!(flags & CreationFlagsBits::hidden))
        {
            if (flags & CreationFlagsBits::minimized)
                ShowWindow(_platform->hwnd, SW_MINIMIZE);
            else if (flags & CreationFlagsBits::maximized)
                ShowWindow(_platform->hwnd, SW_MAXIMIZE);
            else
                ShowWindow(_platform->hwnd, SW_SHOWNORMAL);
        }
    }

    Window::~Window()
    {
        if (_accessBridge)
        {
            delete _accessBridge;
            _accessBridge = nullptr;
        }
        if (_platform)
        {
            if (_platform->hwnd)
            {
                RemovePropW(_platform->hwnd, L"APP3DWINDOW");
                DestroyWindow(_platform->hwnd);
                _platform->hwnd = nullptr;
            }
            if (_platform->rawInputData)
            {
                free(_platform->rawInputData);
                _platform->rawInputData = nullptr;
                _platform->rawInputSize = 0;
            }

            delete _platform;
            _platform = nullptr;
        }
    }

    void Window::showWindow()
    {
        if (!hidden())
            return;
        ShowWindow(_platform->hwnd, SW_SHOWNORMAL);
        _platform->flags &= ~CreationFlagsBits::hidden;
    }

    void Window::hideWindow()
    {
        if (hidden())
            return;
        ShowWindow(_platform->hwnd, SW_HIDE);
        _platform->flags |= CreationFlagsBits::hidden;
    }

    void Window::title(const std::string &title)
    {
        _platform->title = convertUTF8toUTF16(title);
        SetWindowTextW(_platform->hwnd, (LPCWSTR)_platform->title.c_str());
    }

    void Window::enableFullscreen()
    {
        _platform->flags |= CreationFlagsBits::fullscreen;
        SetWindowLongPtr(_platform->hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
        SetWindowPos(_platform->hwnd, HWND_TOPMOST, 0, 0, platform::env.context->screenWidth,
                     platform::env.context->screenHeight, SWP_SHOWWINDOW);
    }

    void Window::disableFullscreen()
    {
        _platform->flags &= ~CreationFlagsBits::fullscreen;
        SetWindowLongPtr(_platform->hwnd, GWL_STYLE, _platform->style);
        SetWindowPos(_platform->hwnd, HWND_NOTOPMOST, 0, 0, _platform->dimenstions.x, _platform->dimenstions.y,
                     SWP_SHOWWINDOW);
    }

    Point2D Window::cursorPosition() const
    {
        POINT pos;
        if (GetCursorPos(&pos))
        {
            ScreenToClient(_platform->hwnd, &pos);
            return {pos.x, pos.y};
        }
        return {};
    }

    void Window::cursorPosition(Point2D position)
    {

        POINT pos = {position.x, position.y};
        ClientToScreen(_platform->hwnd, &pos);
        SetCursorPos(pos.x, pos.y);
    }

    void Window::showCursor()
    {
        if (!_platform->isCursorHidden)
            return;
        cursorPosition(_platform->savedCursorPos);
        ReleaseCapture();
        ShowCursor(TRUE);
        _platform->isCursorHidden = false;
    }

    void Window::hideCursor()
    {
        if (_platform->isCursorHidden)
            return;
        _platform->savedCursorPos = cursorPosition();
        SetCapture(_platform->hwnd);
        ShowCursor(FALSE);
        ClipCursor(NULL);
        _platform->isCursorHidden = true;
    }

    Point2D Window::windowPos() const
    {
        RECT rect;
        if (GetWindowRect(_platform->hwnd, &rect))
            return {rect.left, rect.top};
        else
            return {0, 0};
    }

    void Window::windowPos(Point2D position)
    {
        SetWindowPos(_platform->hwnd, NULL, position.x, position.y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
    }

    void Window::centerWindowPos()
    {
        if (maximized())
            return;
        RECT workArea = {};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); // Получаем размеры рабочей области экрана

        RECT windowRect;
        GetWindowRect(_platform->hwnd, &windowRect);
        int windowWidth = windowRect.right - windowRect.left;
        int windowHeight = windowRect.bottom - windowRect.top;

        // Вычисляем новые координаты для центрирования окна
        int screenWidth = workArea.right - workArea.left;
        int screenHeight = workArea.bottom - workArea.top;
        int centerX = workArea.left + (screenWidth - windowWidth) / 2;
        int centerY = workArea.top + (screenHeight - windowHeight) / 2;

        if (centerY < workArea.top)
            centerY = workArea.top;

        SetWindowPos(_platform->hwnd, NULL, centerX, centerY, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void Window::minimize() { ShowWindow(_platform->hwnd, SW_MINIMIZE); }

    void Window::maximize() { ShowWindow(_platform->hwnd, maximized() ? SW_RESTORE : SW_MAXIMIZE); }

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
            DispatchMessageW(&msg);
        }

        platform::WindowPlatformData *window = (platform::WindowPlatformData *)GetPropW(hwnd, L"APP3DWINDOW");
        if (!window)
            return;

        if (window->isCursorHidden && window->owner->cursorPosition() != window->savedCursorPos)
        {
            Point2D pos = window->dimenstions / 2.0f;
            window->owner->cursorPosition(pos);
        }
    }

    void waitEventsTimeout(f64 timeout)
    {
        MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD)(timeout * 1e3), QS_ALLINPUT);
        pollEvents();
    }

    void pushEmptyEvent() { PostMessageW(NULL, WM_NULL, 0, 0); }

    f32 getDpi() { return static_cast<f32>(platform::env.context->dpi) / 100.0f; }

    Point2D getWindowSize(const Window &window)
    {
        RECT area;
        GetClientRect(window._platform->hwnd, &area);
        return {area.right, area.bottom};
    }

    std::string getClipboardString(const Window &window)
    {
        HANDLE object;
        int tries = 0;

        // NOTE: Retry clipboard opening a few times as some other application may have it
        //       open and also the Windows Clipboard History reads it after each update
        while (!OpenClipboard(window._platform->hwnd))
        {
            Sleep(1);
            tries++;

            if (tries == 3)
            {
                logError("Failed to open clipboard");
                return "";
            }
        }

        object = GetClipboardData(CF_UNICODETEXT);
        if (!object)
        {
            logError("Failed to get clipboard data");
            CloseClipboard();
            return "";
        }

        c16 *buffer = (c16 *)GlobalLock(object);
        if (!buffer)
        {
            logError("Failed to lock clipboard data");
            CloseClipboard();
            return "";
        }
        platform::env.clipboardData = convertUTF16toUTF8(buffer);
        GlobalUnlock(object);
        CloseClipboard();
        return platform::env.clipboardData;
    }

    void setClipboardString(const Window &window, const std::string &text)
    {
        if (text.empty())
            return;
        int tries = 0;
        int characterCount = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
        HANDLE object = GlobalAlloc(GMEM_MOVEABLE, characterCount * sizeof(WCHAR));
        if (!object)
        {
            logError("Failed to allocate global handle for clipboard");
            return;
        }

        WCHAR *buffer = (WCHAR *)GlobalLock(object);
        if (!buffer)
        {
            logError("Failed to lock global handle");
            GlobalFree(object);
            return;
        }
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buffer, characterCount);
        GlobalUnlock(object);

        // NOTE: Retry clipboard opening a few times as some other application may have it
        //       open and also the Windows Clipboard History reads it after each update
        while (!OpenClipboard(window._platform->hwnd))
        {
            Sleep(1);
            tries++;

            if (tries == 3)
            {
                logError("Failed to open clipboard");
                GlobalFree(object);
                return;
            }
        }

        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, object);
        CloseClipboard();
    }

    Cursor Cursor::create(Cursor::Type type)
    {
        PlatformData *platform;
        switch (type)
        {
            case Type::arrow:
                platform = new PlatformData(LoadCursor(NULL, IDC_ARROW));
                break;
            case Type::ibeam:
                platform = new PlatformData(LoadCursor(NULL, IDC_IBEAM));
                break;
            case Type::crosshair:
                platform = new PlatformData(LoadCursor(NULL, IDC_CROSS));
                break;
            case Type::hand:
                platform = new PlatformData(LoadCursor(NULL, IDC_HAND));
                break;
            case Type::resizeEW:
                platform = new PlatformData(LoadCursor(NULL, IDC_SIZEWE));
                break;
            case Type::resizeNS:
                platform = new PlatformData(LoadCursor(NULL, IDC_SIZENS));
                break;
            case Type::resizeNESW:
                platform = new PlatformData(LoadCursor(NULL, IDC_SIZENESW));
                break;
            case Type::resizeNWSE:
                platform = new PlatformData(LoadCursor(NULL, IDC_SIZENWSE));
                break;
            case Type::resizeAll:
                platform = new PlatformData(LoadCursor(NULL, IDC_SIZEALL));
                break;
            case Type::notAllowed:
                platform = new PlatformData(LoadCursor(NULL, IDC_NO));
                break;
            default:
                platform = nullptr;
                break;
        }
        return Cursor(platform);
    }

    Cursor *Cursor::defaultCursor()
    {
        static Cursor cursor = Cursor::create(Cursor::Type::arrow);
        return &cursor;
    }

    void Cursor::assign() { SetCursor(_platform->cursor); }

} // namespace window