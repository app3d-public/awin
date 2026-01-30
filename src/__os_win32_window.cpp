#include <acul/string/string.hpp>
#include <awin/native_access.hpp>
#include <awin/window.hpp>
#include <shlobj.h>
#include <windef.h>
#include <windowsx.h>
#include "env.hpp"
#include "win32_pd.hpp"

namespace awin
{
    struct Cursor::Platform
    {
        HCURSOR cursor = NULL;
    };

    bool Cursor::valid() const { return _pd->cursor != NULL; }

    namespace platform
    {
        Context ctx;

        struct Win32WindowData final : WindowData
        {
            acul::u16string title;
            DWORD style;
            DWORD ex_style;
            HWND hwnd;
            WCHAR high_surrogate;
            acul::point2D<i32> saved_cursor_pos{0, 0};
            bool cursor_tracked{false};
            bool raw_input{false};
            LPBYTE raw_input_data{nullptr};
            UINT raw_input_size{0};
        };

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
            if (flags & WindowFlagBits::fullscreen) style |= WS_POPUP;
            if (flags & WindowFlagBits::snapped) style |= WS_SYSMENU;
            if (flags & WindowFlagBits::minimize_box) style |= WS_MINIMIZEBOX;
            if (flags & WindowFlagBits::maximize_box) style |= WS_MAXIMIZEBOX;
            if (flags & WindowFlagBits::resizable) style |= WS_THICKFRAME;
            if (flags & WindowFlagBits::decorated) style |= WS_CAPTION;
            return style;
        }

        static io::KeyMode get_key_mods()
        {
            io::KeyMode mods;
            if (GetKeyState(VK_SHIFT) & 0x8000) mods |= io::KeyModeBits::shift;
            if (GetKeyState(VK_CONTROL) & 0x8000) mods |= io::KeyModeBits::control;
            if (GetKeyState(VK_MENU) & 0x8000) mods |= io::KeyModeBits::alt;
            if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) mods |= io::KeyModeBits::super;
            if (GetKeyState(VK_CAPITAL) & 1) mods |= io::KeyModeBits::caps_lock;
            if (GetKeyState(VK_NUMLOCK) & 1) mods |= io::KeyModeBits::num_lock;
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

        void on_focus_kill(Win32WindowData *wd)
        {
            if (!wd) return;
            wd->focused = false;
            acul::events::dispatch_event_group<FocusEvent>(event_registry.focus, wd->owner, false);
            if (!wd->raw_input) return;
            const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_REMOVE, NULL};
            if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                AWIN_LOG_ERROR("[Win32] Failed to remove raw input device. Error code: %lu", GetLastError());
            else
                wd->raw_input = false;
        }

        LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            auto *window = (Win32WindowData *)GetPropW(hwnd, L"AWIN");
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
                    window = reinterpret_cast<Win32WindowData *>(GetPropW(hwnd, L"AWIN"));
                    ctx.dpi = GetDpiForWindow(hwnd);
                    ctx.frame.x = GetSystemMetricsForDpi(SM_CXFRAME, ctx.dpi);
                    ctx.frame.y = GetSystemMetricsForDpi(SM_CYFRAME, ctx.dpi);
                    ctx.padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, ctx.dpi);

                    if (!wParam || !window || (window->flags & WindowFlagBits::decorated) ||
                        (window->flags & WindowFlagBits::fullscreen))
                        break;

                    NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)lParam;
                    add_frame_to_client_area(params->rgrc, is_maximized(hwnd), -1);
                    return 0;
                }
                case WM_CREATE:
                {
                    CREATESTRUCT *create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
                    window = reinterpret_cast<Win32WindowData *>(create_struct->lpCreateParams);
                    if (!window) break;
                    SetPropW(hwnd, L"AWIN", reinterpret_cast<HANDLE>(window));
                    MonitorInfo monitor_info = get_primary_monitor_info();
                    ctx.screen.x = monitor_info.dimensions.x;
                    ctx.screen.y = monitor_info.dimensions.y;
                    if (window->flags & WindowFlagBits::fullscreen)
                    {
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, ctx.screen.x, ctx.screen.y, SWP_SHOWWINDOW);
                        return 0;
                    }

                    if (window->flags & WindowFlagBits::decorated) break;
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
                    if (window->flags & WindowFlagBits::decorated) break;
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
                    Win32NativeEvent event(event_id::nc_hit_test, window->owner, hwnd, uMsg, wParam, lParam, hit);
                    for (const auto &node : *event_registry.nc_hit_test)
                    {
                        node.call(node.ctx, event);
                        if (event.lResult != -1) return event.lResult;
                    }
                    break;
                }
                case WM_NCLBUTTONDOWN:
                {
                    if (event_registry.ncl_mouse_down)
                    {
                        POINT cursor_point;
                        GetCursorPos(&cursor_point);
                        ScreenToClient(hwnd, &cursor_point);
                        if (cursor_point.y > 0 && cursor_point.y < ctx.frame.y + ctx.padding) break;
                        Win32NativeEvent event(event_id::nc_mouse_down, window->owner, hwnd, uMsg, wParam, lParam);
                        for (const auto &node : *event_registry.ncl_mouse_down)
                        {
                            node.call(node.ctx, event);
                            if (event.lResult != -1) return event.lResult;
                        }
                        break;
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
                    acul::events::dispatch_event_group<awin::MouseClickEvent>(event_registry.mouse_click, window->owner,
                                                                              button, action);
                    break;
                }
                case WM_WINDOWPOSCHANGED:
                {
                    auto *wp = reinterpret_cast<WINDOWPOS *>(lParam);
                    if ((wp->flags & 0x8000) && IsIconic(window->hwnd)) on_focus_kill(window);
                    break;
                }
                case WM_SETFOCUS:
                {
                    window->focused = true;
                    acul::events::dispatch_event_group<FocusEvent>(event_registry.focus, window->owner, true);
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_INPUTSINK, hwnd};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        AWIN_LOG_ERROR("[Win32] Failed to register raw input device. Error code: %lu", GetLastError());
                    else
                        window->raw_input = true;
                    break;
                }
                case WM_KILLFOCUS:
                    on_focus_kill(window);
                    break;
                case WM_CHAR:
                case WM_SYSCHAR:
                {
                    if (IS_HIGH_SURROGATE(wParam))
                        window->high_surrogate = wParam;
                    else if (IS_LOW_SURROGATE(wParam))
                    {
                        if (window->high_surrogate)
                        {
                            u32 codepoint = (((window->high_surrogate - 0xD800) << 10) | (wParam - 0xDC00)) + 0x10000;
                            window->high_surrogate = 0;
                            acul::events::dispatch_event_group<CharInputEvent>(event_registry.char_input, window->owner,
                                                                               codepoint);
                        }
                    }
                    else
                        acul::events::dispatch_event_group<CharInputEvent>(event_registry.char_input, window->owner,
                                                                           wParam);

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
                    acul::events::dispatch_event_group<CharInputEvent>(event_registry.char_input, window->owner,
                                                                       wParam);
                    return 0;
                }
                case WM_SYSCOMMAND:
                {
                    switch (wParam & 0xfff0)
                    {
                        case SC_SCREENSAVE:
                        case SC_MONITORPOWER:
                        {
                            if (window->flags & WindowFlagBits::fullscreen)
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
                    const auto mods = get_key_mods();
                    io::Key key = io::Key::unknown;
                    switch (wParam)
                    {
                        case VK_MENU:
                            key = (HIWORD(lParam) & KF_EXTENDED) ? io::Key::ralt : io::Key::lalt;
                            break;
                        case VK_SHIFT:
                        {
                            if (action == io::KeyPressState::release)
                            {
                                // HACK: Release both Shift keys on Shift up event, as when both
                                //       are pressed the first release does not dispatch any event
                                input_key(window, io::Key::lshift, action, mods);
                                input_key(window, io::Key::rshift, action, mods);
                            }
                            else
                                key = (HIWORD(lParam) & KF_EXTENDED) ? io::Key::rshift : io::Key::lshift;
                            break;
                        }
                        case VK_CONTROL:
                        {
                            if (HIWORD(lParam) & KF_EXTENDED)
                                key = io::Key::rcontrol;
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
                                key = io::Key::lcontrol;
                            }
                            break;
                        }
                        case VK_PROCESSKEY:
                            // IME notifies that keys have been filtered by setting the
                            // virtual key-code to VK_PROCESSKEY
                            break;
                        case VK_SNAPSHOT:
                            // HACK: Key down is not reported for the Print Screen key
                            input_key(window, key, io::KeyPressState::press, mods);
                            input_key(window, key, io::KeyPressState::release, mods);
                            break;
                        default:
                        {
                            key = ctx.keymap.find(wParam);
                            break;
                        }
                    }

                    if (key != io::Key::unknown) input_key(window, key, action, mods);
                    // Prevent Alt to call Menu Behavior
                    if (wParam == VK_MENU) return 0;
                    break;
                }
                case WM_MOUSEMOVE:
                {
                    if (!window->cursor_tracked)
                    {
                        TRACKMOUSEEVENT tme;
                        ZeroMemory(&tme, sizeof(tme));
                        tme.cbSize = sizeof(tme);
                        tme.dwFlags = TME_LEAVE;
                        tme.hwndTrack = window->hwnd;
                        TrackMouseEvent(&tme);
                        window->cursor_tracked = true;
                        acul::events::dispatch_event_group<MouseEnterEvent>(event_registry.mouse_enter, window->owner,
                                                                            true);
                    }
                    acul::events::dispatch_event_group<PosEvent>(
                        event_registry.mouse_move, event_id::mouse_move, window->owner,
                        acul::point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    return 0;
                }
                case WM_MOUSELEAVE:
                    window->cursor_tracked = false;
                    acul::events::dispatch_event_group<MouseEnterEvent>(event_registry.mouse_enter, window->owner,
                                                                        false);
                    return 0;
                case WM_MOUSEWHEEL:
                    acul::events::dispatch_event_group<ScrollEvent>(event_registry.scroll, window->owner, 0,
                                                                    (SHORT)HIWORD(wParam) / (f64)WHEEL_DELTA);
                    return 0;
                case WM_MOUSEHWHEEL:
                {
                    // This message is only sent on Windows Vista and later
                    // NOTE: The X-axis is inverted for consistency with macOS and X11
                    acul::events::dispatch_event_group<ScrollEvent>(event_registry.scroll, window->owner,
                                                                    -((SHORT)HIWORD(wParam) / (f64)WHEEL_DELTA), 0);
                    return 0;
                }
                case WM_SIZE:
                {
                    acul::point2D<i32> dimenstions(LOWORD(lParam), HIWORD(lParam));
                    if (!(window->flags & WindowFlagBits::hidden))
                    {
                        bool want_min = (wParam == SIZE_MINIMIZED);
                        bool want_max = (wParam == SIZE_MAXIMIZED);
                        if ((window->flags & WindowFlagBits::minimized) != want_min)
                        {
                            if (want_min)
                            {
                                window->flags |= WindowFlagBits::minimized;
                                dimenstions = {0, 0};
                            }
                            else
                                window->flags &= ~WindowFlagBits::minimized;
                            acul::events::dispatch_event_group<StateEvent>(event_registry.minimize, event_id::minimize,
                                                                           window->owner, want_min);
                        }
                        if ((window->flags & WindowFlagBits::maximized) != want_max)
                        {
                            if (want_max)
                                window->flags |= WindowFlagBits::maximized;
                            else
                                window->flags &= ~WindowFlagBits::maximized;
                            acul::events::dispatch_event_group<StateEvent>(event_registry.maximize, event_id::maximize,
                                                                           window->owner, want_max);
                        }
                    }
                    if (dimenstions != window->dimenstions)
                    {
                        window->dimenstions = dimenstions;
                        acul::events::dispatch_event_group<PosEvent>(event_registry.resize, event_id::resize,
                                                                     window->owner, dimenstions);
                    }
                    return 0;
                }
                case WM_MOVE:
                    acul::events::dispatch_event_group<PosEvent>(
                        event_registry.move, event_id::move, window->owner,
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
                    acul::events::dispatch_event_group<DpiChangedEvent>(event_registry.dpi_changed, window->owner,
                                                                        xscale, yscale);
                    break;
                }
                case WM_SETCURSOR:
                {
                    if (LOWORD(lParam) == HTCLIENT)
                    {
                        if (window->cursor->valid())
                            window->cursor->assign(nullptr);
                        else
                            SetCursor(LoadCursor(NULL, IDC_ARROW));
                        return TRUE;
                    }
                    break;
                }
                case WM_INPUT:
                {
                    if (!window->raw_input) break;
                    UINT dw_size;
                    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dw_size, sizeof(RAWINPUTHEADER));
                    if (dw_size > window->raw_input_size)
                    {
                        acul::release(window->raw_input_data);
                        window->raw_input_data = acul::alloc_n<BYTE>(dw_size);
                        window->raw_input_size = dw_size;
                    }
                    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, window->raw_input_data, &dw_size,
                                        sizeof(RAWINPUTHEADER)) != dw_size)
                    {
                        AWIN_LOG_ERROR("[Win32] GetRawInputData does not return correct size");
                        break;
                    }
                    RAWINPUT *raw = (RAWINPUT *)window->raw_input_data;

                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        acul::point2D<i32> delta{raw->data.mouse.lLastX, raw->data.mouse.lLastY};
                        acul::events::dispatch_event_group<PosEvent>(event_registry.mouse_move_delta,
                                                                     event_id::mouse_move_delta, window->owner, delta);
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
            AWIN_LOG_INFO("[Win32] Destroying platform");
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
                AWIN_LOG_WARN("[Win32] Failed to set process dpi awareness context");
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
                AWIN_LOG_WARN("[Win32] Failed to load window icon");
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
            return {.work = {monitor_info.rcWork.right, monitor_info.rcWork.bottom},
                    .dimensions = {monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                                   monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top}};
        }
        else
            return {};
    }

    Window::Window(const acul::string &title, i32 width, i32 height, WindowFlags flags)
        : _data(acul::alloc<platform::Win32WindowData>())
    {
        auto *wd = (platform::Win32WindowData *)_data;
        wd->owner = this;
        wd->title = acul::utf8_to_utf16(title);
        wd->dimenstions = {width == -1 ? CW_USEDEFAULT : width, height == -1 ? CW_USEDEFAULT : height};
        wd->flags = flags;
        wd->style = platform::get_window_style(flags);
        wd->ex_style = WS_EX_APPWINDOW;
        wd->hwnd = nullptr;
        wd->cursor = &platform::env.default_cursor;
        wd->hwnd = CreateWindowExW(wd->ex_style, platform::ctx.win32_class.lpszClassName, (LPCWSTR)wd->title.c_str(),
                                   wd->style & ~WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, wd->dimenstions.x,
                                   wd->dimenstions.y, nullptr, nullptr, platform::ctx.instance, (LPVOID)wd);

        if (!wd->hwnd) throw acul::runtime_error("Failed to create window");
        if (!(flags & WindowFlagBits::hidden))
        {
            if (flags & WindowFlagBits::minimized)
                ShowWindow(wd->hwnd, SW_MINIMIZE);
            else if (flags & WindowFlagBits::maximized)
                ShowWindow(wd->hwnd, SW_MAXIMIZE);
            else
                ShowWindow(wd->hwnd, SW_SHOWNORMAL);
        }
        AWIN_LOG_INFO("[Win32] Created Window descriptor: %p", wd->hwnd);
    }

    void Window::destroy()
    {
        auto *wd = (platform::Win32WindowData *)_data;
        if (wd->raw_input_data)
        {
            acul::release(wd->raw_input_data);
            wd->raw_input_data = nullptr;
            wd->raw_input_size = 0;
        }

        if (wd->hwnd)
        {
            RemovePropW(wd->hwnd, L"AWIN");
            AWIN_LOG_INFO("[Win32] Destroying Window descriptor: %p", wd->hwnd);
            HWND hwnd = wd->hwnd;
            DestroyWindow(hwnd);
            wd->hwnd = nullptr;
        }

        CoUninitialize();
    }

    void Window::show_window()
    {
        if (!hidden()) return;
        WINDOWPLACEMENT placement = {sizeof(WINDOWPLACEMENT)};
        auto *wd = (platform::Win32WindowData *)_data;
        GetWindowPlacement(wd->hwnd, &placement);
        placement.showCmd = wd->flags & WindowFlagBits::maximized ? SW_SHOWMAXIMIZED : SW_NORMAL;
        SetWindowPlacement(wd->hwnd, &placement);
        wd->flags &= ~WindowFlagBits::hidden;
    }

    void Window::hide_window()
    {
        if (hidden()) return;
        auto *wd = (platform::Win32WindowData *)_data;
        ShowWindow(wd->hwnd, SW_HIDE);
        wd->flags |= WindowFlagBits::hidden;
    }

    acul::string Window::title() const
    {
        auto *wd = (platform::Win32WindowData *)_data;
        return acul::utf16_to_utf8(wd->title);
    }

    void Window::title(const acul::string &title)
    {
        auto *wd = (platform::Win32WindowData *)_data;
        wd->title = acul::utf8_to_utf16(title);
        SetWindowTextW(wd->hwnd, (LPCWSTR)wd->title.c_str());
    }

    void Window::enable_fullscreen()
    {
        auto *wd = (platform::Win32WindowData *)_data;
        wd->flags |= WindowFlagBits::fullscreen;
        SetWindowLongPtr(wd->hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
        SetWindowPos(wd->hwnd, HWND_TOPMOST, 0, 0, platform::ctx.screen.x, platform::ctx.screen.y, SWP_SHOWWINDOW);
    }

    void Window::disable_fullscreen()
    {
        auto *wd = (platform::Win32WindowData *)_data;
        wd->flags &= ~WindowFlagBits::fullscreen;
        SetWindowLongPtr(wd->hwnd, GWL_STYLE, wd->style);
        SetWindowPos(wd->hwnd, HWND_NOTOPMOST, 0, 0, wd->dimenstions.x, wd->dimenstions.y, SWP_SHOWWINDOW);
    }

    acul::point2D<i32> Window::cursor_position() const
    {
        POINT pos;
        if (GetCursorPos(&pos))
        {
            auto *wd = (platform::Win32WindowData *)_data;
            ScreenToClient(wd->hwnd, &pos);
            return {pos.x, pos.y};
        }
        return {};
    }

    void Window::cursor_position(acul::point2D<i32> position)
    {
        auto *wd = (platform::Win32WindowData *)_data;
        POINT pos = {position.x, position.y};
        ClientToScreen(wd->hwnd, &pos);
        SetCursorPos(pos.x, pos.y);
    }

    void Window::show_cursor()
    {
        auto *wd = (platform::Win32WindowData *)_data;
        if (!wd->is_cursor_hidden) return;
        cursor_position(wd->saved_cursor_pos);
        ReleaseCapture();
        ShowCursor(TRUE);
        wd->is_cursor_hidden = false;
    }

    void Window::hide_cursor()
    {
        auto *wd = (platform::Win32WindowData *)_data;
        if (wd->is_cursor_hidden) return;
        wd->saved_cursor_pos = cursor_position();
        SetCapture(wd->hwnd);
        ShowCursor(FALSE);
        ClipCursor(NULL);
        wd->is_cursor_hidden = true;
    }

    acul::point2D<i32> Window::position() const
    {
        RECT rect;
        auto *wd = (platform::Win32WindowData *)_data;
        if (GetWindowRect(wd->hwnd, &rect))
            return {rect.left, rect.top};
        else
            return {0, 0};
    }

    void Window::position(acul::point2D<i32> position)
    {
        WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
        auto *wd = (platform::Win32WindowData *)_data;
        GetWindowPlacement(wd->hwnd, &wp);

        acul::point2D<i32> dimensions =
            wd->flags & WindowFlagBits::decorated ? get_window_size(*this) : native_access::get_full_client_size(*this);
        wp.rcNormalPosition.left = position.x;
        wp.rcNormalPosition.top = position.y;
        wp.rcNormalPosition.right = position.x + dimensions.x;
        wp.rcNormalPosition.bottom = position.y + dimensions.y;
        SetWindowPlacement(wd->hwnd, &wp);
    }

    void Window::center_window()
    {
        RECT work_area = {};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);

        RECT window_rect;
        auto *wd = (platform::Win32WindowData *)_data;
        GetWindowRect(wd->hwnd, &window_rect);
        acul::point2D<int> dimenstions{window_rect.right - window_rect.left, window_rect.bottom - window_rect.top};
        acul::point2D<int> screen{work_area.right - work_area.left, work_area.bottom - work_area.top};
        acul::point2D<int> center{work_area.left + (screen.x - dimenstions.x) / 2,
                                  work_area.top + (screen.y - dimenstions.y) / 2};

        if (center.y < work_area.top) center.y = work_area.top;
        SetWindowPos(wd->hwnd, NULL, center.x, center.y, dimenstions.x, dimenstions.y, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void Window::update_resize_limit() {}; // No need

    void Window::minimize()
    {
        auto *wd = (platform::Win32WindowData *)_data;
        ShowWindow(wd->hwnd, SW_MINIMIZE);
    }

    void Window::maximize()
    {
        auto *wd = (platform::Win32WindowData *)_data;
        ShowWindow(wd->hwnd, maximized() ? SW_RESTORE : SW_MAXIMIZE);
    }

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

        auto *window = (platform::Win32WindowData *)GetPropW(hwnd, L"AWIN");
        if (!window) return;
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

    f32 get_dpi(const Window &) { return static_cast<f32>(platform::ctx.dpi) / 96.0f; }

    acul::point2D<i32> get_window_size(const Window &window)
    {
        RECT area;
        GetClientRect(native_access::get_hwnd(window), &area);
        return {area.right, area.bottom};
    }

    acul::string get_clipboard_string(const Window &window)
    {
        HANDLE object;
        int tries = 0;

        // NOTE: Retry clipboard opening a few times as some other application may have it
        //       open and also the Windows Clipboard History reads it after each update
        HWND hwnd = native_access::get_hwnd(window);
        while (!OpenClipboard(hwnd))
        {
            Sleep(1);
            tries++;

            if (tries == 3)
            {
                AWIN_LOG_ERROR("[Win32] Failed to open clipboard");
                return "";
            }
        }

        object = GetClipboardData(CF_UNICODETEXT);
        if (!object)
        {
            AWIN_LOG_ERROR("[Win32] Failed to get clipboard data");
            CloseClipboard();
            return "";
        }

        c16 *buffer = (c16 *)GlobalLock(object);
        if (!buffer)
        {
            AWIN_LOG_ERROR("[Win32] Failed to lock clipboard data. Error code: %lu", GetLastError());
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
            AWIN_LOG_ERROR("[Win32] Failed to allocate global handle for clipboard. Error code: %lu", GetLastError());
            return;
        }

        WCHAR *buffer = (WCHAR *)GlobalLock(object);
        if (!buffer)
        {
            AWIN_LOG_ERROR("[Win32] Failed to lock global handle. Error code: %lu", GetLastError());
            GlobalFree(object);
            return;
        }
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buffer, character_count);
        GlobalUnlock(object);

        // NOTE: Retry clipboard opening a few times as some other application may have it
        //       open and also the Windows Clipboard History reads it after each update
        HWND hwnd = native_access::get_hwnd(window);
        while (!OpenClipboard(hwnd))
        {
            Sleep(1);
            tries++;

            if (tries == 3)
            {
                AWIN_LOG_ERROR("[Win32] Failed to open clipboard");
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
        Cursor::Platform *pd;
        switch (type)
        {
            case Type::arrow:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_ARROW));
                break;
            case Type::ibeam:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_IBEAM));
                break;
            case Type::crosshair:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_CROSS));
                break;
            case Type::hand:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_HAND));
                break;
            case Type::resize_ew:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_SIZEWE));
                break;
            case Type::resize_ns:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_SIZENS));
                break;
            case Type::resize_nesw:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_SIZENESW));
                break;
            case Type::resize_nwse:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_SIZENWSE));
                break;
            case Type::resize_all:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_SIZEALL));
                break;
            case Type::not_allowed:
                pd = acul::alloc<Cursor::Platform>(LoadCursor(NULL, IDC_NO));
                break;
            default:
                pd = NULL;
                break;
        }
        return Cursor(pd);
    }

    void Cursor::assign(Window *window) { SetCursor(_pd->cursor); }

    HWND native_access::get_hwnd(const Window &window)
    {
        auto *wd = (platform::Win32WindowData *)get_window_data(window);
        return wd->hwnd;
    }

    acul::point2D<i32> native_access::get_full_client_size(const Window &window)
    {
        RECT clent_rect;
        auto *wd = (platform::Win32WindowData *)get_window_data(window);
        GetClientRect(wd->hwnd, &clent_rect);
        platform::add_frame_to_client_area(&clent_rect, wd->flags & WindowFlagBits::maximized, 1);
        return {clent_rect.right - clent_rect.left, clent_rect.bottom - clent_rect.top};
    }

} // namespace awin