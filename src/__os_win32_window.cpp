#include <acul/log.hpp>
#include <acul/string/string.hpp>
#include <awin/window.hpp>
#include <shlobj.h>
#include <windef.h>
#include <windowsx.h>

namespace awin
{
    namespace platform
    {
        Win32Ctx ctx;

        static bool isMaximized(HWND hwnd)
        {
            WINDOWPLACEMENT placement = {0};
            placement.length = sizeof(WINDOWPLACEMENT);
            if (GetWindowPlacement(hwnd, &placement)) return placement.showCmd == SW_SHOWMAXIMIZED;
            return false;
        }

        DWORD getWindowStyle(CreationFlags flags)
        {
            DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            if (flags & CreationFlagsBits::fullscreen) style |= WS_POPUP;
            if (flags & CreationFlagsBits::snapped) style |= WS_SYSMENU;
            if (flags & CreationFlagsBits::minimizebox) style |= WS_MINIMIZEBOX;
            if (flags & CreationFlagsBits::maximizebox) style |= WS_MAXIMIZEBOX;
            if (flags & CreationFlagsBits::resizable) style |= WS_THICKFRAME;
            if (flags & CreationFlagsBits::decorated) style |= WS_CAPTION;
            return style;
        }

        static io::KeyMode getKeyMods()
        {
            io::KeyMode mods;
            if (GetKeyState(VK_SHIFT) & 0x8000) mods |= io::KeyModeBits::shift;
            if (GetKeyState(VK_CONTROL) & 0x8000) mods |= io::KeyModeBits::control;
            if (GetKeyState(VK_MENU) & 0x8000) mods |= io::KeyModeBits::alt;
            if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) mods |= io::KeyModeBits::super;
            if (GetKeyState(VK_CAPITAL) & 1) mods |= io::KeyModeBits::capsLock;
            if (GetKeyState(VK_NUMLOCK) & 1) mods |= io::KeyModeBits::numLock;
            return mods;
        }

        LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            WindowData *window = (WindowData *)GetPropW(hwnd, L"APP3DWINDOW");
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
                    window = reinterpret_cast<WindowData *>(GetPropW(hwnd, L"APP3DWINDOW"));
                    ctx.dpi = GetDpiForWindow(hwnd);
                    ctx.frameX = GetSystemMetricsForDpi(SM_CXFRAME, ctx.dpi);
                    ctx.frameY = GetSystemMetricsForDpi(SM_CYFRAME, ctx.dpi);
                    ctx.padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, ctx.dpi);

                    if (!wParam || !window || (window->flags & CreationFlagsBits::decorated) ||
                        (window->flags & CreationFlagsBits::fullscreen))
                        break;

                    NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)lParam;
                    RECT *clentRect = params->rgrc;
                    clentRect->right -= ctx.frameX + ctx.padding;
                    clentRect->left += ctx.frameX + ctx.padding;
                    clentRect->bottom -= ctx.frameY + ctx.padding;

                    if (isMaximized(hwnd)) clentRect->top += ctx.frameY + ctx.padding;
                    return 0;
                }
                case WM_CREATE:
                {
                    CREATESTRUCT *createStruct = reinterpret_cast<CREATESTRUCT *>(lParam);
                    window = reinterpret_cast<WindowData *>(createStruct->lpCreateParams);
                    if (!window) break;
                    SetPropW(hwnd, L"APP3DWINDOW", reinterpret_cast<HANDLE>(window));
                    MonitorInfo monitorInfo = getPrimaryMonitorInfo();
                    ctx.screenWidth = monitorInfo.width;
                    ctx.screenHeight = monitorInfo.height;
                    if (window->flags & CreationFlagsBits::fullscreen)
                    {
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, ctx.screenWidth, ctx.screenHeight, SWP_SHOWWINDOW);
                        return 0;
                    }

                    if (window->flags & CreationFlagsBits::decorated) break;
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
                case WM_ERASEBKGND:
                {
                    HDC hdc = (HDC)wParam;
                    HBRUSH hBrush = CreateSolidBrush(RGB(35, 35, 35));
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    FillRect(hdc, &rect, hBrush);
                    DeleteObject(hBrush);
                    return TRUE;
                }

                case WM_NCHITTEST:
                {
                    if (window->flags & CreationFlagsBits::decorated) break;
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
                    if (cursorPoint.y > 0 && cursorPoint.y < ctx.frameY + ctx.padding) return HTTOP;
                    if (!eventRegistry.NCHitTest) break;
                    Win32NativeEvent event(event_id::NCHitTest, window->owner, hwnd, uMsg, wParam, lParam, hit);
                    eventRegistry.NCHitTest->invoke(event);
                    return event.lResult;
                }
                case WM_NCLBUTTONDOWN:
                {
                    if (eventRegistry.NCLMouseDown)
                    {
                        POINT cursorPoint;
                        GetCursorPos(&cursorPoint);
                        ScreenToClient(hwnd, &cursorPoint);
                        if (cursorPoint.y > 0 && cursorPoint.y < ctx.frameY + ctx.padding) break;
                        Win32NativeEvent event(event_id::NCMouseDown, window->owner, hwnd, uMsg, wParam, lParam);
                        eventRegistry.NCLMouseDown->invoke(event);
                        if (event.lResult != -1) return event.lResult;
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
                    dispatchWindowEvent(eventRegistry.mouseClick, window->owner, button, action);
                    break;
                }
                case WM_SETFOCUS:
                {
                    window->focused = true;
                    dispatchWindowEvent(eventRegistry.focus, window->owner, true);
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_INPUTSINK, hwnd};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        logError("Failed to register RAWINPUTDEVICE");
                    else
                        window->backend.rawInput = true;
                    break;
                }
                case WM_KILLFOCUS:
                {
                    if (!window) break;
                    window->focused = false;
                    dispatchWindowEvent(eventRegistry.focus, window->owner, false);
                    if (!window->backend.rawInput) break;
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_REMOVE, NULL};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        logError("Failed to remove raw input device");
                    else
                        window->backend.rawInput = false;
                    break;
                }
                case WM_CHAR:
                case WM_SYSCHAR:
                {
                    if (IS_HIGH_SURROGATE(wParam))
                        window->backend.highSurrogate = wParam;
                    else if (IS_LOW_SURROGATE(wParam))
                    {
                        if (window->backend.highSurrogate)
                        {
                            u32 codepoint =
                                (((window->backend.highSurrogate - 0xD800) << 10) | (wParam - 0xDC00)) + 0x10000;
                            window->backend.highSurrogate = 0;
                            dispatchWindowEvent(eventRegistry.charInput, window->owner, codepoint);
                        }
                    }
                    else
                        dispatchWindowEvent(eventRegistry.charInput, window->owner, wParam);

                    if (uMsg == WM_SYSCHAR) break;
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
                    dispatchWindowEvent(eventRegistry.charInput, window->owner, wParam);
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
                                //       are pressed the first release does not dispatch any event
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
                            auto it = ctx.keymap.find(wParam);
                            key = it != ctx.keymap.end() ? it->second : io::Key::kUnknown;
                            break;
                        }
                    }

                    if (key != io::Key::kUnknown) inputKey(window, key, action, mods);
                    // Prevent Alt to call Menu Behavior
                    if (wParam == VK_MENU) return 0;
                    break;
                }
                case WM_MOUSEMOVE:
                {
                    if (!window->backend.cursorTracked)
                    {
                        TRACKMOUSEEVENT tme;
                        ZeroMemory(&tme, sizeof(tme));
                        tme.cbSize = sizeof(tme);
                        tme.dwFlags = TME_LEAVE;
                        tme.hwndTrack = window->backend.hwnd;
                        TrackMouseEvent(&tme);
                        window->backend.cursorTracked = true;
                        dispatchWindowEvent(eventRegistry.mouseEnter, window->owner, true);
                    }
                    dispatchWindowEvent(eventRegistry.mouseMoveAbs, event_id::mouseMoveAbs, window->owner,
                                        acul::point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    return 0;
                }
                case WM_MOUSELEAVE:
                    window->backend.cursorTracked = false;
                    dispatchWindowEvent(eventRegistry.mouseEnter, window->owner, false);
                    return 0;
                case WM_MOUSEWHEEL:
                    dispatchWindowEvent(eventRegistry.scroll, window->owner, 0,
                                        (SHORT)HIWORD(wParam) / (f64)WHEEL_DELTA);
                    return 0;
                case WM_MOUSEHWHEEL:
                {
                    // This message is only sent on Windows Vista and later
                    // NOTE: The X-axis is inverted for consistency with macOS and X11
                    dispatchWindowEvent(eventRegistry.scroll, window->owner,
                                        -((SHORT)HIWORD(wParam) / (f64)WHEEL_DELTA), 0);
                    return 0;
                }
                case WM_SIZE:
                {
                    acul::point2D<i32> dimenstions(LOWORD(lParam), HIWORD(lParam));

                    if ((window->flags & CreationFlagsBits::minimized) != (wParam == SIZE_MINIMIZED))
                    {
                        if (!(window->flags & CreationFlagsBits::preinitialized))
                            window->flags ^= CreationFlagsBits::minimized;
                        if (window->flags & CreationFlagsBits::minimized) dimenstions = {0, 0};
                        dispatchWindowEvent(eventRegistry.minimize, event_id::minimize, window->owner,
                                            window->flags & CreationFlagsBits::minimized);
                    }
                    if ((window->flags & CreationFlagsBits::maximized) != (wParam == SIZE_MAXIMIZED))
                    {
                        if (!(window->flags & CreationFlagsBits::preinitialized))
                            window->flags ^= CreationFlagsBits::maximized;
                        dispatchWindowEvent(eventRegistry.maximize, event_id::maximize, window->owner,
                                            window->flags & CreationFlagsBits::maximized);
                    }
                    if (dimenstions != window->dimenstions)
                    {
                        window->dimenstions = dimenstions;
                        dispatchWindowEvent(eventRegistry.resize, event_id::resize, window->owner, dimenstions);
                    }
                    return 0;
                }
                case WM_MOVE:
                    dispatchWindowEvent(eventRegistry.move, event_id::move, window->owner,
                                        acul::point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    break;
                case WM_GETMINMAXINFO:
                {
                    if (!window) break;
                    MINMAXINFO *mmi = (MINMAXINFO *)lParam;
                    mmi->ptMinTrackSize.x = window->resizeLimit.x;
                    mmi->ptMinTrackSize.y = window->resizeLimit.y;
                    return 0;
                }
                case WM_DPICHANGED:
                {
                    const float xscale = HIWORD(wParam) / 96.0f;
                    const float yscale = LOWORD(wParam) / 96.0f;
                    dispatchWindowEvent(eventRegistry.dpiChanged, window->owner, xscale, yscale);
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
                    if (!window->backend.rawInput) break;
                    UINT dwSize;
                    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                    if (dwSize > window->backend.rawInputSize)
                    {
                        acul::release(window->backend.rawInputData);
                        window->backend.rawInputData = acul::alloc_n<BYTE>(dwSize);
                        window->backend.rawInputSize = dwSize;
                    }
                    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, window->backend.rawInputData, &dwSize,
                                        sizeof(RAWINPUTHEADER)) != dwSize)
                    {
                        logError("GetRawInputData does not return correct size");
                        break;
                    }
                    RAWINPUT *raw = (RAWINPUT *)window->backend.rawInputData;

                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        acul::point2D<i32> delta{raw->data.mouse.lLastX, raw->data.mouse.lLastY};
                        dispatchWindowEvent(eventRegistry.mouseMove, event_id::mouseMove, window->owner, delta);
                    }
                    return 0;
                }
                case WM_CLOSE:
                    window->readyToClose = true;
                    return TRUE;
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
            if (ctx.instance)
            {
                if (ctx.win32class.hIcon) DestroyIcon(ctx.win32class.hIcon);
                UnregisterClassW(ctx.win32class.lpszClassName, ctx.instance);
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
            ctx.instance = GetModuleHandleW(nullptr);
            ctx.win32class = {sizeof(ctx.win32class)};
            ctx.win32class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            ctx.win32class.lpfnWndProc = wndProc;
            ctx.win32class.hInstance = ctx.instance;
            ctx.win32class.lpszClassName = L"APP3DWINDOWLIB";
            ctx.win32class.hCursor = LoadCursor(NULL, IDC_ARROW);
            ctx.win32class.hIcon = LoadIconW(ctx.instance, L"APP_ICON");
            if (!ctx.win32class.hIcon)
            {
                logWarn("Failed to load window icon");
                ctx.win32class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            }
            if (!RegisterClassExW(&ctx.win32class)) return false;
            // Init platform for using COM objects
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            return !FAILED(hr);
        }
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

    Window::Window(const acul::string &title, i32 width, i32 height, CreationFlags flags) : _platform(nullptr)
    {
        _platform.owner = this;
        _platform.backend.title = acul::utf8_to_utf16(title);
        _platform.dimenstions = {width == -1 ? CW_USEDEFAULT : width, height == -1 ? CW_USEDEFAULT : height};
        _platform.flags = flags;
        if (flags & CreationFlagsBits::fullscreen || flags & CreationFlagsBits::maximized ||
            flags & CreationFlagsBits::maximized)
            _platform.flags |= CreationFlagsBits::preinitialized;
        _platform.backend.style = platform::getWindowStyle(flags);
        _platform.backend.exStyle = WS_EX_APPWINDOW;
        _platform.backend.hwnd = nullptr;
        _platform.cursor = Cursor::defaultCursor();

        _platform.backend.hwnd = CreateWindowExW(
            _platform.backend.exStyle, platform::ctx.win32class.lpszClassName, (LPCWSTR)_platform.backend.title.c_str(),
            _platform.backend.style & ~WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, _platform.dimenstions.x,
            _platform.dimenstions.y, nullptr, nullptr, platform::ctx.instance, (LPVOID)&_platform);

        if (!_platform.backend.hwnd) throw acul::runtime_error("Failed to create window");
        if (!(flags & CreationFlagsBits::hidden))
        {
            if (flags & CreationFlagsBits::minimized)
                ShowWindow(_platform.backend.hwnd, SW_MINIMIZE);
            else if (flags & CreationFlagsBits::maximized)
                ShowWindow(_platform.backend.hwnd, SW_MAXIMIZE);
            else
                ShowWindow(_platform.backend.hwnd, SW_SHOWNORMAL);
        }
        logInfo("Created Window descriptor: %p", _platform.backend.hwnd);
    }

    void Window::destroy()
    {
        if (_platform.backend.rawInputData)
        {
            acul::release(_platform.backend.rawInputData);
            _platform.backend.rawInputData = nullptr;
            _platform.backend.rawInputSize = 0;
        }

        if (_platform.backend.hwnd)
        {
            RemovePropW(_platform.backend.hwnd, L"APP3DWINDOW");
            logInfo("Destroying Window descriptor: %p", _platform.backend.hwnd);
            HWND hwnd = _platform.backend.hwnd;
            DestroyWindow(hwnd);
            _platform.backend.hwnd = nullptr;
        }

        CoUninitialize();
    }

    void Window::showWindow()
    {
        if (!hidden()) return;
        WINDOWPLACEMENT placement = {sizeof(WINDOWPLACEMENT)};
        GetWindowPlacement(_platform.backend.hwnd, &placement);
        placement.showCmd = _platform.flags & CreationFlagsBits::maximized ? SW_SHOWMAXIMIZED : SW_NORMAL;
        SetWindowPlacement(_platform.backend.hwnd, &placement);
        _platform.flags &= ~CreationFlagsBits::hidden;
        _platform.flags &= ~CreationFlagsBits::preinitialized;
    }

    void Window::hideWindow()
    {
        if (hidden()) return;
        ShowWindow(_platform.backend.hwnd, SW_HIDE);
        _platform.flags |= CreationFlagsBits::hidden;
    }

    void Window::title(const acul::string &title)
    {
        _platform.backend.title = acul::utf8_to_utf16(title);
        SetWindowTextW(_platform.backend.hwnd, (LPCWSTR)_platform.backend.title.c_str());
    }

    void Window::enableFullscreen()
    {
        _platform.flags |= CreationFlagsBits::fullscreen;
        SetWindowLongPtr(_platform.backend.hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
        SetWindowPos(_platform.backend.hwnd, HWND_TOPMOST, 0, 0, platform::ctx.screenWidth, platform::ctx.screenHeight,
                     SWP_SHOWWINDOW);
    }

    void Window::disableFullscreen()
    {
        _platform.flags &= ~CreationFlagsBits::fullscreen;
        SetWindowLongPtr(_platform.backend.hwnd, GWL_STYLE, _platform.backend.style);
        SetWindowPos(_platform.backend.hwnd, HWND_NOTOPMOST, 0, 0, _platform.dimenstions.x, _platform.dimenstions.y,
                     SWP_SHOWWINDOW);
    }

    acul::point2D<i32> Window::cursorPosition() const
    {
        POINT pos;
        if (GetCursorPos(&pos))
        {
            ScreenToClient(_platform.backend.hwnd, &pos);
            return {pos.x, pos.y};
        }
        return {};
    }

    void Window::cursorPosition(acul::point2D<i32> position)
    {

        POINT pos = {position.x, position.y};
        ClientToScreen(_platform.backend.hwnd, &pos);
        SetCursorPos(pos.x, pos.y);
    }

    void Window::showCursor()
    {
        if (!_platform.isCursorHidden) return;
        cursorPosition(_platform.backend.savedCursorPos);
        ReleaseCapture();
        ShowCursor(TRUE);
        _platform.isCursorHidden = false;
    }

    void Window::hideCursor()
    {
        if (_platform.isCursorHidden) return;
        _platform.backend.savedCursorPos = cursorPosition();
        SetCapture(_platform.backend.hwnd);
        ShowCursor(FALSE);
        ClipCursor(NULL);
        _platform.isCursorHidden = true;
    }

    acul::point2D<i32> Window::windowPos() const
    {
        RECT rect;
        if (GetWindowRect(_platform.backend.hwnd, &rect))
            return {rect.left, rect.top};
        else
            return {0, 0};
    }

    void Window::windowPos(acul::point2D<i32> position)
    {
        WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
        GetWindowPlacement(_platform.backend.hwnd, &wp);
        auto dimenstions = getWindowSize(*this);
        wp.rcNormalPosition.left = position.x;
        wp.rcNormalPosition.top = position.y;
        wp.rcNormalPosition.right = position.x + dimenstions.x;
        wp.rcNormalPosition.bottom = position.y + dimenstions.y;
        SetWindowPlacement(_platform.backend.hwnd, &wp);
    }

    void Window::centerWindowPos()
    {
        RECT workArea = {};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); // Получаем размеры рабочей области экрана

        RECT windowRect;
        GetWindowRect(_platform.backend.hwnd, &windowRect);
        int windowWidth = windowRect.right - windowRect.left;
        int windowHeight = windowRect.bottom - windowRect.top;

        int screenWidth = workArea.right - workArea.left;
        int screenHeight = workArea.bottom - workArea.top;
        int centerX = workArea.left + (screenWidth - windowWidth) / 2;
        int centerY = workArea.top + (screenHeight - windowHeight) / 2;

        if (centerY < workArea.top) centerY = workArea.top;

        SetWindowPos(_platform.backend.hwnd, NULL, centerX, centerY, windowWidth, windowHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void Window::minimize() { ShowWindow(_platform.backend.hwnd, SW_MINIMIZE); }

    void Window::maximize() { ShowWindow(_platform.backend.hwnd, maximized() ? SW_RESTORE : SW_MAXIMIZE); }

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

        platform::WindowData *window = (platform::WindowData *)GetPropW(hwnd, L"APP3DWINDOW");
        if (!window) return;

        if (window->isCursorHidden && window->owner->cursorPosition() != window->backend.savedCursorPos)
        {
            acul::point2D<i32> pos = window->dimenstions / 2.0f;
            window->owner->cursorPosition(pos);
        }
    }

    void waitEventsTimeout()
    {
        if (platform::env.timeout > WINDOW_TIMEOUT_INF)
        {
            MsgWaitForMultipleObjects(0, NULL, FALSE, platform::env.timeout * 1e3, QS_ALLINPUT);
            platform::env.timeout = WINDOW_TIMEOUT_INF;
        }
        else
            WaitMessage();
        pollEvents();
    }

    void pushEmptyEvent() { PostMessageW(NULL, WM_NULL, 0, 0); }

    f32 getDpi() { return static_cast<f32>(platform::ctx.dpi) / 96.0f; }

    acul::point2D<i32> getWindowSize(const Window &window)
    {
        RECT area;
        GetClientRect(platform::native_access::getHWND(window), &area);
        return {area.right, area.bottom};
    }

    acul::string getClipboardString(const Window &window)
    {
        HANDLE object;
        int tries = 0;

        // NOTE: Retry clipboard opening a few times as some other application may have it
        //       open and also the Windows Clipboard History reads it after each update
        HWND hwnd = platform::native_access::getHWND(window);
        while (!OpenClipboard(hwnd))
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
        platform::env.clipboardData = acul::utf16_to_utf8(buffer);
        GlobalUnlock(object);
        CloseClipboard();
        return platform::env.clipboardData;
    }

    void setClipboardString(const Window &window, const acul::string &text)
    {
        if (text.empty()) return;
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
        HWND hwnd = platform::native_access::getHWND(window);
        while (!OpenClipboard(hwnd))
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
        switch (type)
        {
            case Type::arrow:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_ARROW))};
            case Type::ibeam:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_IBEAM))};
            case Type::crosshair:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_CROSS))};
            case Type::hand:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_HAND))};
            case Type::resizeEW:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZEWE))};
            case Type::resizeNS:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZENS))};
            case Type::resizeNESW:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZENESW))};
            case Type::resizeNWSE:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZENWSE))};
            case Type::resizeAll:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZEALL))};
            case Type::notAllowed:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_NO))};
            default:
                return platform::Win32Cursor();
        }
    }

    Cursor *Cursor::defaultCursor()
    {
        static Cursor cursor = Cursor::create(Cursor::Type::arrow);
        return &cursor;
    }

    void Cursor::assign() { SetCursor(_platform.cursor); }

    HWND platform::native_access::getHWND(const Window &window) { return window._platform.backend.hwnd; }

} // namespace awin