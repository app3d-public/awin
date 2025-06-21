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
        struct Win32Ctx
        {
            HINSTANCE instance;
            WNDCLASSEXW win32_class;
            DWORD thread_id;
            int padding;
            acul::point2D<int> frame, screen;
            UINT dpi;
            acul::map<i16, io::Key> keymap{{VK_SPACE, io::Key::Space},
                                           {VK_OEM_7, io::Key::Apostroph},
                                           {VK_OEM_COMMA, io::Key::Comma},
                                           {VK_OEM_MINUS, io::Key::Minus},
                                           {VK_OEM_PERIOD, io::Key::Period},
                                           {VK_OEM_2, io::Key::Slash},
                                           {0x30, io::Key::D0},
                                           {0x31, io::Key::D1},
                                           {0x32, io::Key::D2},
                                           {0x33, io::Key::D3},
                                           {0x34, io::Key::D4},
                                           {0x35, io::Key::D5},
                                           {0x36, io::Key::D6},
                                           {0x37, io::Key::D7},
                                           {0x38, io::Key::D8},
                                           {0x39, io::Key::D9},
                                           {VK_OEM_1, io::Key::Semicolon},
                                           {VK_OEM_PLUS, io::Key::Equal},
                                           {0x41, io::Key::A},
                                           {0x42, io::Key::B},
                                           {0x43, io::Key::C},
                                           {0x44, io::Key::D},
                                           {0x45, io::Key::E},
                                           {0x46, io::Key::F},
                                           {0x47, io::Key::G},
                                           {0x48, io::Key::H},
                                           {0x49, io::Key::I},
                                           {0x4A, io::Key::J},
                                           {0x4B, io::Key::K},
                                           {0x4C, io::Key::L},
                                           {0x4D, io::Key::M},
                                           {0x4E, io::Key::N},
                                           {0x4F, io::Key::O},
                                           {0x50, io::Key::P},
                                           {0x51, io::Key::Q},
                                           {0x52, io::Key::R},
                                           {0x53, io::Key::S},
                                           {0x54, io::Key::T},
                                           {0x55, io::Key::U},
                                           {0x56, io::Key::V},
                                           {0x57, io::Key::W},
                                           {0x58, io::Key::X},
                                           {0x59, io::Key::Y},
                                           {0x5A, io::Key::Z},
                                           {VK_OEM_4, io::Key::LeftBrace},
                                           {VK_OEM_5, io::Key::Backslash},
                                           {VK_OEM_6, io::Key::RightBrace},
                                           {VK_OEM_3, io::Key::GraveAccent},
                                           {VK_ESCAPE, io::Key::Escape},
                                           {VK_RETURN, io::Key::Enter},
                                           {VK_TAB, io::Key::Tab},
                                           {VK_BACK, io::Key::Backspace},
                                           {VK_INSERT, io::Key::Insert},
                                           {VK_DELETE, io::Key::Delete},
                                           {VK_RIGHT, io::Key::Right},
                                           {VK_LEFT, io::Key::Left},
                                           {VK_DOWN, io::Key::Down},
                                           {VK_UP, io::Key::Up},
                                           {VK_PRIOR, io::Key::PageUp},
                                           {VK_NEXT, io::Key::PageDown},
                                           {VK_HOME, io::Key::Home},
                                           {VK_END, io::Key::End},
                                           {VK_CAPITAL, io::Key::CapsLock},
                                           {VK_SCROLL, io::Key::ScrollLock},
                                           {VK_NUMLOCK, io::Key::NumLock},
                                           {VK_SNAPSHOT, io::Key::PrintScreen},
                                           {VK_PAUSE, io::Key::Pause},
                                           {VK_F1, io::Key::F1},
                                           {VK_F2, io::Key::F2},
                                           {VK_F3, io::Key::F3},
                                           {VK_F4, io::Key::F4},
                                           {VK_F5, io::Key::F5},
                                           {VK_F6, io::Key::F6},
                                           {VK_F7, io::Key::F7},
                                           {VK_F8, io::Key::F8},
                                           {VK_F9, io::Key::F9},
                                           {VK_F10, io::Key::F10},
                                           {VK_F11, io::Key::F11},
                                           {VK_F12, io::Key::F12},
                                           {VK_F13, io::Key::F13},
                                           {VK_F14, io::Key::F14},
                                           {VK_F15, io::Key::F15},
                                           {VK_F16, io::Key::F16},
                                           {VK_F17, io::Key::F17},
                                           {VK_F18, io::Key::F18},
                                           {VK_F19, io::Key::F19},
                                           {VK_F20, io::Key::F20},
                                           {VK_F21, io::Key::F21},
                                           {VK_F22, io::Key::F22},
                                           {VK_F23, io::Key::F23},
                                           {VK_F24, io::Key::F24},
                                           {VK_NUMPAD0, io::Key::KP0},
                                           {VK_NUMPAD1, io::Key::KP1},
                                           {VK_NUMPAD2, io::Key::KP2},
                                           {VK_NUMPAD3, io::Key::KP3},
                                           {VK_NUMPAD4, io::Key::KP4},
                                           {VK_NUMPAD5, io::Key::KP5},
                                           {VK_NUMPAD6, io::Key::KP6},
                                           {VK_NUMPAD7, io::Key::KP7},
                                           {VK_NUMPAD8, io::Key::KP8},
                                           {VK_NUMPAD9, io::Key::KP9},
                                           {VK_DECIMAL, io::Key::KPDecimal},
                                           {VK_DIVIDE, io::Key::KPDivide},
                                           {VK_MULTIPLY, io::Key::KPMultiply},
                                           {VK_SUBTRACT, io::Key::KPSubtract},
                                           {VK_ADD, io::Key::KPAdd},
                                           {VK_RETURN, io::Key::KPEnter},
                                           {VK_OEM_PLUS, io::Key::KPEqual},
                                           {VK_LSHIFT, io::Key::LeftShift},
                                           {VK_LCONTROL, io::Key::LeftControl},
                                           {VK_LMENU, io::Key::LeftAlt},
                                           {VK_LWIN, io::Key::LeftSuper},
                                           {VK_RSHIFT, io::Key::RightShift},
                                           {VK_RCONTROL, io::Key::RightControl},
                                           {VK_RMENU, io::Key::RightAlt},
                                           {VK_RWIN, io::Key::RightSuper},
                                           {VK_APPS, io::Key::Menu}};
        } ctx;

        static bool is_maximized(HWND hwnd)
        {
            WINDOWPLACEMENT placement = {0};
            placement.length = sizeof(WINDOWPLACEMENT);
            if (GetWindowPlacement(hwnd, &placement)) return placement.showCmd == SW_SHOWMAXIMIZED;
            return false;
        }

        DWORD get_window_style(WindowFlags flags)
        {
            DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            if (flags & WindowFlagBits::Fullscreen) style |= WS_POPUP;
            if (flags & WindowFlagBits::Snapped) style |= WS_SYSMENU;
            if (flags & WindowFlagBits::MinimizeBox) style |= WS_MINIMIZEBOX;
            if (flags & WindowFlagBits::MaximizeBox) style |= WS_MAXIMIZEBOX;
            if (flags & WindowFlagBits::Resizable) style |= WS_THICKFRAME;
            if (flags & WindowFlagBits::Decorated) style |= WS_CAPTION;
            return style;
        }

        static io::KeyMode get_key_mods()
        {
            io::KeyMode mods;
            if (GetKeyState(VK_SHIFT) & 0x8000) mods |= io::KeyModeBits::Shift;
            if (GetKeyState(VK_CONTROL) & 0x8000) mods |= io::KeyModeBits::Control;
            if (GetKeyState(VK_MENU) & 0x8000) mods |= io::KeyModeBits::Alt;
            if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) mods |= io::KeyModeBits::Super;
            if (GetKeyState(VK_CAPITAL) & 1) mods |= io::KeyModeBits::CapsLock;
            if (GetKeyState(VK_NUMLOCK) & 1) mods |= io::KeyModeBits::NumLock;
            return mods;
        }

        void add_frame_to_client_area(RECT *area, bool is_maximized, int multiplier)
        {
            int border_x = (platform::ctx.frame.x + platform::ctx.padding) * multiplier;
            int border_y = (platform::ctx.frame.y + platform::ctx.padding) * multiplier;

            area->left -= border_x;
            area->right += border_x;
            area->bottom += border_y;

            if (is_maximized) area->top -= border_y;
        }

        LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            WindowData *window = (WindowData *)GetPropW(hwnd, L"AWIN");
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
                    window = reinterpret_cast<WindowData *>(GetPropW(hwnd, L"AWIN"));
                    ctx.dpi = GetDpiForWindow(hwnd);
                    ctx.frame.x = GetSystemMetricsForDpi(SM_CXFRAME, ctx.dpi);
                    ctx.frame.y = GetSystemMetricsForDpi(SM_CYFRAME, ctx.dpi);
                    ctx.padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, ctx.dpi);

                    if (!wParam || !window || (window->flags & WindowFlagBits::Decorated) ||
                        (window->flags & WindowFlagBits::Fullscreen))
                        break;

                    NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)lParam;
                    add_frame_to_client_area(params->rgrc, is_maximized(hwnd), -1);
                    return 0;
                }
                case WM_CREATE:
                {
                    CREATESTRUCT *create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
                    window = reinterpret_cast<WindowData *>(create_struct->lpCreateParams);
                    if (!window) break;
                    SetPropW(hwnd, L"AWIN", reinterpret_cast<HANDLE>(window));
                    MonitorInfo monitor_info = get_primary_monitor_info();
                    ctx.screen.x = monitor_info.width;
                    ctx.screen.y = monitor_info.height;
                    if (window->flags & WindowFlagBits::Fullscreen)
                    {
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, ctx.screen.x, ctx.screen.y, SWP_SHOWWINDOW);
                        return 0;
                    }

                    if (window->flags & WindowFlagBits::Decorated) break;
                    RECT size_rect;
                    GetWindowRect(hwnd, &size_rect);

                    // Inform the application of the frame change to force redrawing with the new
                    // client area that is extended into the title bar
                    SetWindowPos(hwnd, NULL, size_rect.left, size_rect.top, size_rect.right - size_rect.left,
                                 size_rect.bottom - size_rect.top, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
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
                    if (window->flags & WindowFlagBits::Decorated) break;
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
                    POINT cursor_point = {0};
                    cursor_point.x = LOWORD(lParam);
                    cursor_point.y = HIWORD(lParam);
                    ScreenToClient(hwnd, &cursor_point);
                    if (cursor_point.y > 0 && cursor_point.y < ctx.frame.y + ctx.padding) return HTTOP;
                    if (!event_registry.nc_hit_test) break;
                    Win32NativeEvent event(event_id::NCHitTest, window->owner, hwnd, uMsg, wParam, lParam, hit);
                    event_registry.nc_hit_test->invoke(event);
                    return event.lResult;
                }
                case WM_NCLBUTTONDOWN:
                {
                    if (event_registry.ncl_mouse_down)
                    {
                        POINT cursor_point;
                        GetCursorPos(&cursor_point);
                        ScreenToClient(hwnd, &cursor_point);
                        if (cursor_point.y > 0 && cursor_point.y < ctx.frame.y + ctx.padding) break;
                        Win32NativeEvent event(event_id::NCMouseDown, window->owner, hwnd, uMsg, wParam, lParam);
                        event_registry.ncl_mouse_down->invoke(event);
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
                            button = io::MouseKey::Left;
                            action = io::KeyPressState::Press;
                            break;
                        case WM_LBUTTONUP:
                            button = io::MouseKey::Left;
                            action = io::KeyPressState::Release;
                            break;
                        case WM_RBUTTONDOWN:
                            button = io::MouseKey::Right;
                            action = io::KeyPressState::Press;
                            break;
                        case WM_RBUTTONUP:
                            button = io::MouseKey::Right;
                            action = io::KeyPressState::Release;
                            break;
                        case WM_MBUTTONDOWN:
                            button = io::MouseKey::Middle;
                            action = io::KeyPressState::Press;
                            break;
                        case WM_MBUTTONUP:
                            button = io::MouseKey::Middle;
                            action = io::KeyPressState::Release;
                            break;
                        default:
                            button = io::MouseKey::Unknown;
                            action = io::KeyPressState::Release;
                            break;
                    };
                    dispatch_window_event(event_registry.mouse_click, window->owner, button, action);
                    break;
                }
                case WM_SETFOCUS:
                {
                    window->focused = true;
                    dispatch_window_event(event_registry.focus, window->owner, true);
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_INPUTSINK, hwnd};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        LOG_ERROR("Failed to register RAWINPUTDEVICE");
                    else
                        window->backend.raw_input = true;
                    break;
                }
                case WM_KILLFOCUS:
                {
                    if (!window) break;
                    window->focused = false;
                    dispatch_window_event(event_registry.focus, window->owner, false);
                    if (!window->backend.raw_input) break;
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_REMOVE, NULL};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        LOG_ERROR("Failed to remove raw input device");
                    else
                        window->backend.raw_input = false;
                    break;
                }
                case WM_CHAR:
                case WM_SYSCHAR:
                {
                    if (IS_HIGH_SURROGATE(wParam))
                        window->backend.high_surrogate = wParam;
                    else if (IS_LOW_SURROGATE(wParam))
                    {
                        if (window->backend.high_surrogate)
                        {
                            u32 codepoint =
                                (((window->backend.high_surrogate - 0xD800) << 10) | (wParam - 0xDC00)) + 0x10000;
                            window->backend.high_surrogate = 0;
                            dispatch_window_event(event_registry.char_input, window->owner, codepoint);
                        }
                    }
                    else
                        dispatch_window_event(event_registry.char_input, window->owner, wParam);

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
                    dispatch_window_event(event_registry.char_input, window->owner, wParam);
                    return 0;
                }
                case WM_SYSCOMMAND:
                {
                    switch (wParam & 0xfff0)
                    {
                        case SC_SCREENSAVE:
                        case SC_MONITORPOWER:
                        {
                            if (window->flags & WindowFlagBits::Fullscreen)
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
                        (HIWORD(lParam) & KF_UP) ? io::KeyPressState::Release : io::KeyPressState::Press;
                    const auto mods = get_key_mods();
                    io::Key key = io::Key::Unknown;
                    switch (wParam)
                    {
                        case VK_MENU:
                            key = (HIWORD(lParam) & KF_EXTENDED) ? io::Key::RightAlt : io::Key::LeftAlt;
                            break;
                        case VK_SHIFT:
                        {
                            if (action == io::KeyPressState::Release)
                            {
                                // HACK: Release both Shift keys on Shift up event, as when both
                                //       are pressed the first release does not dispatch any event
                                input_key(window, io::Key::LeftShift, action, mods);
                                input_key(window, io::Key::RightShift, action, mods);
                            }
                            else
                                key = (HIWORD(lParam) & KF_EXTENDED) ? io::Key::RightShift : io::Key::LeftShift;
                            break;
                        }
                        case VK_CONTROL:
                        {
                            if (HIWORD(lParam) & KF_EXTENDED)
                                key = io::Key::RightControl;
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
                                key = io::Key::LeftControl;
                            }
                            break;
                        }
                        case VK_PROCESSKEY:
                            // IME notifies that keys have been filtered by setting the
                            // virtual key-code to VK_PROCESSKEY
                            break;
                        case VK_SNAPSHOT:
                            // HACK: Key down is not reported for the Print Screen key
                            input_key(window, key, io::KeyPressState::Press, mods);
                            input_key(window, key, io::KeyPressState::Release, mods);
                            break;
                        default:
                        {
                            auto it = ctx.keymap.find(wParam);
                            key = it != ctx.keymap.end() ? it->second : io::Key::Unknown;
                            break;
                        }
                    }

                    if (key != io::Key::Unknown) input_key(window, key, action, mods);
                    // Prevent Alt to call Menu Behavior
                    if (wParam == VK_MENU) return 0;
                    break;
                }
                case WM_MOUSEMOVE:
                {
                    if (!window->backend.cursor_tracked)
                    {
                        TRACKMOUSEEVENT tme;
                        ZeroMemory(&tme, sizeof(tme));
                        tme.cbSize = sizeof(tme);
                        tme.dwFlags = TME_LEAVE;
                        tme.hwndTrack = window->backend.hwnd;
                        TrackMouseEvent(&tme);
                        window->backend.cursor_tracked = true;
                        dispatch_window_event(event_registry.mouse_enter, window->owner, true);
                    }
                    dispatch_window_event(event_registry.mouse_move_abs, event_id::MouseMoveAbs, window->owner,
                                          acul::point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    return 0;
                }
                case WM_MOUSELEAVE:
                    window->backend.cursor_tracked = false;
                    dispatch_window_event(event_registry.mouse_enter, window->owner, false);
                    return 0;
                case WM_MOUSEWHEEL:
                    dispatch_window_event(event_registry.scroll, window->owner, 0,
                                          (SHORT)HIWORD(wParam) / (f64)WHEEL_DELTA);
                    return 0;
                case WM_MOUSEHWHEEL:
                {
                    // This message is only sent on Windows Vista and later
                    // NOTE: The X-axis is inverted for consistency with macOS and X11
                    dispatch_window_event(event_registry.scroll, window->owner,
                                          -((SHORT)HIWORD(wParam) / (f64)WHEEL_DELTA), 0);
                    return 0;
                }
                case WM_SIZE:
                {
                    acul::point2D<i32> dimenstions(LOWORD(lParam), HIWORD(lParam));
                    if (!(window->flags & WindowFlagBits::Hidden))
                    {
                        bool want_min = (wParam == SIZE_MINIMIZED);
                        bool want_max = (wParam == SIZE_MAXIMIZED);
                        if ((window->flags & WindowFlagBits::Minimized) != want_min)
                        {
                            if (want_min)
                            {
                                window->flags |= WindowFlagBits::Minimized;
                                dimenstions = {0, 0};
                            }
                            else
                                window->flags &= ~WindowFlagBits::Minimized;
                            dispatch_window_event(event_registry.minimize, event_id::Minimize, window->owner, want_min);
                        }
                        if ((window->flags & WindowFlagBits::Maximized) != want_max)
                        {
                            if (want_max)
                                window->flags |= WindowFlagBits::Maximized;
                            else
                                window->flags &= ~WindowFlagBits::Maximized;
                            dispatch_window_event(event_registry.maximize, event_id::Maximize, window->owner, want_max);
                        }
                    }
                    if (dimenstions != window->dimenstions)
                    {
                        window->dimenstions = dimenstions;
                        dispatch_window_event(event_registry.resize, event_id::Resize, window->owner, dimenstions);
                    }
                    return 0;
                }
                case WM_MOVE:
                    dispatch_window_event(event_registry.move, event_id::Move, window->owner,
                                          acul::point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    break;
                case WM_GETMINMAXINFO:
                {
                    if (!window) break;
                    MINMAXINFO *mmi = (MINMAXINFO *)lParam;
                    mmi->ptMinTrackSize.x = window->resize_limit.x;
                    mmi->ptMinTrackSize.y = window->resize_limit.y;
                    return 0;
                }
                case WM_DPICHANGED:
                {
                    const float xscale = HIWORD(wParam) / 96.0f;
                    const float yscale = LOWORD(wParam) / 96.0f;
                    dispatch_window_event(event_registry.dpi_changed, window->owner, xscale, yscale);
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
                    if (!window->backend.raw_input) break;
                    UINT dw_size;
                    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dw_size, sizeof(RAWINPUTHEADER));
                    if (dw_size > window->backend.raw_input_size)
                    {
                        acul::release(window->backend.raw_input_data);
                        window->backend.raw_input_data = acul::alloc_n<BYTE>(dw_size);
                        window->backend.raw_input_size = dw_size;
                    }
                    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, window->backend.raw_input_data, &dw_size,
                                        sizeof(RAWINPUTHEADER)) != dw_size)
                    {
                        LOG_ERROR("GetRawInputData does not return correct size");
                        break;
                    }
                    RAWINPUT *raw = (RAWINPUT *)window->backend.raw_input_data;

                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        acul::point2D<i32> delta{raw->data.mouse.lLastX, raw->data.mouse.lLastY};
                        dispatch_window_event(event_registry.mouse_move, event_id::MouseMove, window->owner, delta);
                    }
                    return 0;
                }
                case WM_CLOSE:
                    window->ready_to_close = true;
                    return TRUE;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
                default:
                    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
            }
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }

        void destroy_platform()
        {
            if (ctx.instance)
            {
                if (ctx.win32_class.hIcon) DestroyIcon(ctx.win32_class.hIcon);
                UnregisterClassW(ctx.win32_class.lpszClassName, ctx.instance);
            }
        }

        void init_timer() { QueryPerformanceFrequency((LARGE_INTEGER *)&env.timer.frequency); }

        u64 get_time_value()
        {
            u64 value;
            QueryPerformanceCounter((LARGE_INTEGER *)&value);
            return value;
        }

        u64 get_time_frequency() { return env.timer.frequency; }

        bool init_platform()
        {
            if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
                LOG_WARN("Failed to set process dpi awareness context");
            ctx.instance = GetModuleHandleW(nullptr);
            ctx.thread_id = GetCurrentThreadId();
            ctx.win32_class = {sizeof(ctx.win32_class)};
            ctx.win32_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            ctx.win32_class.lpfnWndProc = wnd_proc;
            ctx.win32_class.hInstance = ctx.instance;
            ctx.win32_class.lpszClassName = L"AWINLIB";
            ctx.win32_class.hCursor = LoadCursor(NULL, IDC_ARROW);
            ctx.win32_class.hIcon = LoadIconW(ctx.instance, L"APP_ICON");
            if (!ctx.win32_class.hIcon)
            {
                LOG_WARN("Failed to load window icon");
                ctx.win32_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            }
            if (!RegisterClassExW(&ctx.win32_class)) return false;
            // Init platform for using COM objects
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            return !FAILED(hr);
        }
    } // namespace platform

    MonitorInfo get_primary_monitor_info()
    {
        HMONITOR hMonitor = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monitor_info = {sizeof(monitor_info)};
        if (GetMonitorInfoW(hMonitor, &monitor_info))
        {
            return {.xpos = monitor_info.rcWork.right,
                    .ypos = monitor_info.rcWork.bottom,
                    .width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                    .height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top};
        }
        else
            return {};
    }

    Window::Window(const acul::string &title, i32 width, i32 height, WindowFlags flags) : _platform(nullptr)
    {
        _platform.owner = this;
        _platform.backend.title = acul::utf8_to_utf16(title);
        _platform.dimenstions = {width == -1 ? CW_USEDEFAULT : width, height == -1 ? CW_USEDEFAULT : height};
        _platform.flags = flags;
        _platform.backend.style = platform::get_window_style(flags);
        _platform.backend.ex_style = WS_EX_APPWINDOW;
        _platform.backend.hwnd = nullptr;
        _platform.cursor = Cursor::default_cursor();

        _platform.backend.hwnd =
            CreateWindowExW(_platform.backend.ex_style, platform::ctx.win32_class.lpszClassName,
                            (LPCWSTR)_platform.backend.title.c_str(), _platform.backend.style & ~WS_VISIBLE,
                            CW_USEDEFAULT, CW_USEDEFAULT, _platform.dimenstions.x, _platform.dimenstions.y, nullptr,
                            nullptr, platform::ctx.instance, (LPVOID)&_platform);

        if (!_platform.backend.hwnd) throw acul::runtime_error("Failed to create window");
        if (!(flags & WindowFlagBits::Hidden))
        {
            if (flags & WindowFlagBits::Minimized)
                ShowWindow(_platform.backend.hwnd, SW_MINIMIZE);
            else if (flags & WindowFlagBits::Maximized)
                ShowWindow(_platform.backend.hwnd, SW_MAXIMIZE);
            else
                ShowWindow(_platform.backend.hwnd, SW_SHOWNORMAL);
        }
        LOG_INFO("Created Window descriptor: %p", _platform.backend.hwnd);
    }

    void Window::destroy()
    {
        if (_platform.backend.raw_input_data)
        {
            acul::release(_platform.backend.raw_input_data);
            _platform.backend.raw_input_data = nullptr;
            _platform.backend.raw_input_size = 0;
        }

        if (_platform.backend.hwnd)
        {
            RemovePropW(_platform.backend.hwnd, L"AWIN");
            LOG_INFO("Destroying Window descriptor: %p", _platform.backend.hwnd);
            HWND hwnd = _platform.backend.hwnd;
            DestroyWindow(hwnd);
            _platform.backend.hwnd = nullptr;
        }

        CoUninitialize();
    }

    void Window::show_window()
    {
        if (!hidden()) return;
        WINDOWPLACEMENT placement = {sizeof(WINDOWPLACEMENT)};
        GetWindowPlacement(_platform.backend.hwnd, &placement);
        placement.showCmd = _platform.flags & WindowFlagBits::Maximized ? SW_SHOWMAXIMIZED : SW_NORMAL;
        SetWindowPlacement(_platform.backend.hwnd, &placement);
        _platform.flags &= ~WindowFlagBits::Hidden;
    }

    void Window::hide_window()
    {
        if (hidden()) return;
        ShowWindow(_platform.backend.hwnd, SW_HIDE);
        _platform.flags |= WindowFlagBits::Hidden;
    }

    acul::string Window::title() const { return acul::utf16_to_utf8(_platform.backend.title); }

    void Window::title(const acul::string &title)
    {
        _platform.backend.title = acul::utf8_to_utf16(title);
        SetWindowTextW(_platform.backend.hwnd, (LPCWSTR)_platform.backend.title.c_str());
    }

    void Window::enable_fullscreen()
    {
        _platform.flags |= WindowFlagBits::Fullscreen;
        SetWindowLongPtr(_platform.backend.hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
        SetWindowPos(_platform.backend.hwnd, HWND_TOPMOST, 0, 0, platform::ctx.screen.x, platform::ctx.screen.y,
                     SWP_SHOWWINDOW);
    }

    void Window::disable_fullscreen()
    {
        _platform.flags &= ~WindowFlagBits::Fullscreen;
        SetWindowLongPtr(_platform.backend.hwnd, GWL_STYLE, _platform.backend.style);
        SetWindowPos(_platform.backend.hwnd, HWND_NOTOPMOST, 0, 0, _platform.dimenstions.x, _platform.dimenstions.y,
                     SWP_SHOWWINDOW);
    }

    acul::point2D<i32> Window::cursor_position() const
    {
        POINT pos;
        if (GetCursorPos(&pos))
        {
            ScreenToClient(_platform.backend.hwnd, &pos);
            return {pos.x, pos.y};
        }
        return {};
    }

    void Window::cursor_position(acul::point2D<i32> position)
    {

        POINT pos = {position.x, position.y};
        ClientToScreen(_platform.backend.hwnd, &pos);
        SetCursorPos(pos.x, pos.y);
    }

    void Window::show_cursor()
    {
        if (!_platform.is_cursor_hidden) return;
        cursor_position(_platform.backend.saved_cursor_pos);
        ReleaseCapture();
        ShowCursor(TRUE);
        _platform.is_cursor_hidden = false;
    }

    void Window::hide_cursor()
    {
        if (_platform.is_cursor_hidden) return;
        _platform.backend.saved_cursor_pos = cursor_position();
        SetCapture(_platform.backend.hwnd);
        ShowCursor(FALSE);
        ClipCursor(NULL);
        _platform.is_cursor_hidden = true;
    }

    acul::point2D<i32> Window::position() const
    {
        RECT rect;
        if (GetWindowRect(_platform.backend.hwnd, &rect))
            return {rect.left, rect.top};
        else
            return {0, 0};
    }

    void Window::position(acul::point2D<i32> position)
    {
        WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
        GetWindowPlacement(_platform.backend.hwnd, &wp);

        acul::point2D<i32> dimensions = _platform.flags & WindowFlagBits::Decorated
                                            ? get_window_size(*this)
                                            : platform::native_access::get_full_client_size(*this);
        wp.rcNormalPosition.left = position.x;
        wp.rcNormalPosition.top = position.y;
        wp.rcNormalPosition.right = position.x + dimensions.x;
        wp.rcNormalPosition.bottom = position.y + dimensions.y;
        SetWindowPlacement(_platform.backend.hwnd, &wp);
    }

    void Window::center_window()
    {
        RECT work_area = {};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);

        RECT window_rect;
        GetWindowRect(_platform.backend.hwnd, &window_rect);
        acul::point2D<int> dimenstions{window_rect.right - window_rect.left, window_rect.bottom - window_rect.top};
        acul::point2D<int> screen{work_area.right - work_area.left, work_area.bottom - work_area.top};
        acul::point2D<int> center{work_area.left + (screen.x - dimenstions.x) / 2,
                                  work_area.top + (screen.y - dimenstions.y) / 2};

        if (center.y < work_area.top) center.y = work_area.top;
        SetWindowPos(_platform.backend.hwnd, NULL, center.x, center.y, dimenstions.x, dimenstions.y,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void Window::update_resize_limit() {}; // No need

    void Window::minimize() { ShowWindow(_platform.backend.hwnd, SW_MINIMIZE); }

    void Window::maximize() { ShowWindow(_platform.backend.hwnd, maximized() ? SW_RESTORE : SW_MAXIMIZE); }

    void wait_events()
    {
        WaitMessage();
        poll_events();
    }

    void poll_events()
    {
        MSG msg = {};
        HWND hwnd = GetActiveWindow();

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        platform::WindowData *window = (platform::WindowData *)GetPropW(hwnd, L"AWIN");
        if (!window) return;

        if (window->is_cursor_hidden && window->owner->cursor_position() != window->backend.saved_cursor_pos)
        {
            acul::point2D<i32> pos = window->dimenstions / 2.0f;
            window->owner->cursor_position(pos);
        }
    }

    void wait_events_timeout()
    {
        if (platform::env.timeout > WINDOW_TIMEOUT_INF)
        {
            MsgWaitForMultipleObjects(0, NULL, FALSE, platform::env.timeout * 1e3, QS_ALLINPUT);
            platform::env.timeout = WINDOW_TIMEOUT_INF;
        }
        else
            WaitMessage();
        poll_events();
    }

    void push_empty_event() { PostThreadMessageW(platform::ctx.thread_id, WM_NULL, 0, 0); }

    f32 get_dpi() { return static_cast<f32>(platform::ctx.dpi) / 96.0f; }

    acul::point2D<i32> get_window_size(const Window &window)
    {
        RECT area;
        GetClientRect(platform::native_access::get_hwnd(window), &area);
        return {area.right, area.bottom};
    }

    acul::string get_clipboard_string(const Window &window)
    {
        HANDLE object;
        int tries = 0;

        // NOTE: Retry clipboard opening a few times as some other application may have it
        //       open and also the Windows Clipboard History reads it after each update
        HWND hwnd = platform::native_access::get_hwnd(window);
        while (!OpenClipboard(hwnd))
        {
            Sleep(1);
            tries++;

            if (tries == 3)
            {
                LOG_ERROR("Failed to open clipboard");
                return "";
            }
        }

        object = GetClipboardData(CF_UNICODETEXT);
        if (!object)
        {
            LOG_ERROR("Failed to get clipboard data");
            CloseClipboard();
            return "";
        }

        c16 *buffer = (c16 *)GlobalLock(object);
        if (!buffer)
        {
            LOG_ERROR("Failed to lock clipboard data");
            CloseClipboard();
            return "";
        }
        platform::env.clipboard_data = acul::utf16_to_utf8(buffer);
        GlobalUnlock(object);
        CloseClipboard();
        return platform::env.clipboard_data;
    }

    void set_clipboard_string(const Window &window, const acul::string &text)
    {
        if (text.empty()) return;
        int tries = 0;
        int character_count = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
        HANDLE object = GlobalAlloc(GMEM_MOVEABLE, character_count * sizeof(WCHAR));
        if (!object)
        {
            LOG_ERROR("Failed to allocate global handle for clipboard");
            return;
        }

        WCHAR *buffer = (WCHAR *)GlobalLock(object);
        if (!buffer)
        {
            LOG_ERROR("Failed to lock global handle");
            GlobalFree(object);
            return;
        }
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buffer, character_count);
        GlobalUnlock(object);

        // NOTE: Retry clipboard opening a few times as some other application may have it
        //       open and also the Windows Clipboard History reads it after each update
        HWND hwnd = platform::native_access::get_hwnd(window);
        while (!OpenClipboard(hwnd))
        {
            Sleep(1);
            tries++;

            if (tries == 3)
            {
                LOG_ERROR("Failed to open clipboard");
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
            case Type::Arrow:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_ARROW))};
            case Type::Ibeam:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_IBEAM))};
            case Type::Crosshair:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_CROSS))};
            case Type::Hand:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_HAND))};
            case Type::ResizeEW:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZEWE))};
            case Type::ResizeNS:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZENS))};
            case Type::ResizeNESW:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZENESW))};
            case Type::ResizeNWSE:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZENWSE))};
            case Type::ResizeAll:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_SIZEALL))};
            case Type::NotAllowed:
                return {platform::Win32Cursor(LoadCursor(NULL, IDC_NO))};
            default:
                return platform::Win32Cursor();
        }
    }

    void Cursor::assign() { SetCursor(_platform.cursor); }

    HWND platform::native_access::get_hwnd(const Window &window) { return window._platform.backend.hwnd; }

    acul::point2D<i32> platform::native_access::get_full_client_size(const Window &window)
    {
        RECT clent_rect;
        GetClientRect(window._platform.backend.hwnd, &clent_rect);
        platform::add_frame_to_client_area(&clent_rect, window._platform.flags & WindowFlagBits::Maximized, 1);
        return {clent_rect.right - clent_rect.left, clent_rect.bottom - clent_rect.top};
    }

} // namespace awin