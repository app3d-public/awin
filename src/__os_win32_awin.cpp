#include <acul/string/string.hpp>
#include <awin/awin.hpp>
#include <awin/native_access.hpp>
#include <dbt.h>
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
        bool poll_monitors(acul::vector<Monitor> &result);

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
            bool has_background_hint{false};
            COLORREF background_color{RGB(35, 35, 35)};
            u8 resize_flags{0};
        };

        DWORD get_window_style(WindowFlags flags)
        {
            DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            const bool decorated = flags & WindowFlagBits::decorated;
            const bool undecorated_resizable = !decorated && (flags & WindowFlagBits::resizable);
            if ((flags & WindowFlagBits::fullscreen) || (!decorated && !undecorated_resizable)) style |= WS_POPUP;
            if (decorated || (flags & (WindowFlagBits::minimize_box | WindowFlagBits::maximize_box)))
                style |= WS_SYSMENU;
            if (flags & WindowFlagBits::minimize_box) style |= WS_MINIMIZEBOX;
            if (flags & WindowFlagBits::maximize_box) style |= WS_MAXIMIZEBOX;
            if (flags & WindowFlagBits::resizable) style |= WS_THICKFRAME;
            if (decorated) style |= WS_CAPTION;
            return style;
        }

        static bool is_borderless_resizable(const Win32WindowData *window)
        {
            return window && !(window->flags & WindowFlagBits::decorated) &&
                   (window->flags & WindowFlagBits::resizable) && !(window->flags & WindowFlagBits::fullscreen);
        }

        static acul::point2D<i32> get_resize_frame(HWND hwnd)
        {
            const UINT dpi = GetDpiForWindow(hwnd);
            const i32 padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
            return {GetSystemMetricsForDpi(SM_CXFRAME, dpi) + padding,
                    GetSystemMetricsForDpi(SM_CYFRAME, dpi) + padding};
        }

        static void remove_frame_from_client_area(HWND hwnd, RECT *area)
        {
            const auto frame = get_resize_frame(hwnd);
            area->left += frame.x;
            area->right -= frame.x;
            area->bottom -= frame.y;

            if (IsZoomed(hwnd)) area->top += frame.y;
        }

        static acul::point2D<i32> client_to_window_dimensions(i32 width, i32 height, DWORD style, DWORD ex_style,
                                                              WindowFlags flags)
        {
            if (width == CW_USEDEFAULT || height == CW_USEDEFAULT || (flags & WindowFlagBits::fullscreen))
                return {width, height};
            if (!(flags & WindowFlagBits::decorated) && (flags & WindowFlagBits::resizable)) return {width, height};

            RECT rect{0, 0, width, height};
            const UINT dpi = GetDpiForSystem();
            if (!AdjustWindowRectExForDpi(&rect, style, FALSE, ex_style, dpi > 0 ? dpi : 96u))
                AdjustWindowRectEx(&rect, style, FALSE, ex_style);
            return {rect.right - rect.left, rect.bottom - rect.top};
        }

        static LRESULT hit_test_borderless_resize(HWND hwnd, const Win32WindowData *window, LPARAM lParam)
        {
            if (!is_borderless_resizable(window) || IsZoomed(hwnd)) return HTCLIENT;

            RECT rect{};
            if (!GetWindowRect(hwnd, &rect)) return HTCLIENT;

            const auto border = get_resize_frame(hwnd);
            const POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

            const bool left = point.x >= rect.left && point.x < rect.left + border.x;
            const bool right = point.x < rect.right && point.x >= rect.right - border.x;
            const bool top = point.y >= rect.top && point.y < rect.top + border.y;
            const bool bottom = point.y < rect.bottom && point.y >= rect.bottom - border.y;

            if (top && left) return HTTOPLEFT;
            if (top && right) return HTTOPRIGHT;
            if (bottom && left) return HTBOTTOMLEFT;
            if (bottom && right) return HTBOTTOMRIGHT;
            if (left) return HTLEFT;
            if (right) return HTRIGHT;
            if (top) return HTTOP;
            if (bottom) return HTBOTTOM;
            return HTCLIENT;
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

        static void update_primary_screen_cache()
        {
            const Monitor *monitor = get_primary_monitor();
            if (monitor)
            {
                ctx.screen.x = monitor->dimensions.x;
                ctx.screen.y = monitor->dimensions.y;
            }
            if (ctx.screen.x <= 0 || ctx.screen.y <= 0)
            {
                ctx.screen.x = GetSystemMetrics(SM_CXSCREEN);
                ctx.screen.y = GetSystemMetrics(SM_CYSCREEN);
            }
        }

        static const Monitor *find_monitor_by_position(acul::point2D<i32> position)
        {
            for (const auto &monitor : get_monitors())
                if (monitor.position == position) return &monitor;
            return get_primary_monitor();
        }

        static void update_window_monitor_if_needed(Win32WindowData *window)
        {
            if (!window) return;
            if (window->active_monitor &&
                (IsZoomed(window->hwnd) || (window->flags & (WindowFlagBits::maximized | WindowFlagBits::fullscreen))))
                return;

            HMONITOR hmonitor = MonitorFromWindow(window->hwnd, MONITOR_DEFAULTTONEAREST);
            if (!hmonitor) return;

            MONITORINFO info{};
            info.cbSize = sizeof(info);
            if (!GetMonitorInfoW(hmonitor, &info)) return;

            const acul::point2D<i32> monitor_position{info.rcMonitor.left, info.rcMonitor.top};
            if (window->active_monitor && window->active_monitor->position == monitor_position) return;
            update_window_monitor(window, find_monitor_by_position(monitor_position));
        }

        static BOOL CALLBACK refresh_window_monitor_callback(HWND hwnd, LPARAM)
        {
            auto *window = reinterpret_cast<Win32WindowData *>(GetPropW(hwnd, L"AWIN"));
            if (!window) return TRUE;
            window->active_monitor = nullptr;
            update_window_monitor_if_needed(window);
            return TRUE;
        }

        static void refresh_all_window_monitors() { EnumWindows(refresh_window_monitor_callback, 0); }

        static bool fill_window_background(Win32WindowData *window, HDC hdc, const RECT &rect)
        {
            if (!window || !window->has_background_hint || !hdc) return false;
            HBRUSH brush = CreateSolidBrush(window->background_color);
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            return true;
        }

        void on_focus_kill(Win32WindowData *wd)
        {
            if (!wd) return;
            set_window_state_flag(wd->state_flags, WindowStateFlagBits::focused, false);
            set_window_state_flag(wd->state_flags, WindowStateFlagBits::accepts_surface_update, false);
            acul::events::dispatch_event_group<FocusEvent>(g_env->events.focus, wd->owner, false);
            if (!wd->raw_input) return;
            const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_REMOVE, NULL};
            if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                AWIN_LOG_ERROR("[Win32] Failed to remove raw input device. Error code: %lu", GetLastError());
            else wd->raw_input = false;
        }

        LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            auto *window = (Win32WindowData *)GetPropW(hwnd, L"AWIN");
            auto &events = g_env->events;
            switch (uMsg)
            {
                case WM_CREATE:
                {
                    CREATESTRUCT *create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
                    window = reinterpret_cast<Win32WindowData *>(create_struct->lpCreateParams);
                    if (!window) break;
                    SetPropW(hwnd, L"AWIN", reinterpret_cast<HANDLE>(window));
                    ctx.dpi = GetDpiForWindow(hwnd);
                    update_primary_screen_cache();
                    if (window->flags & WindowFlagBits::fullscreen)
                    {
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, ctx.screen.x, ctx.screen.y, SWP_SHOWWINDOW);
                        return 0;
                    }
                    if (is_borderless_resizable(window))
                    {
                        // Force the custom non-client calculation before the first show, so the initial layout
                        // cannot retain a system caption strip.
                        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                    }
                    break;
                }
                case WM_NCCREATE:
                {
                    EnableNonClientDpiScaling(hwnd);
                    CREATESTRUCT *create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
                    window = reinterpret_cast<Win32WindowData *>(create_struct->lpCreateParams);
                    if (window)
                    {
                        window->hwnd = hwnd;
                        SetPropW(hwnd, L"AWIN", reinterpret_cast<HANDLE>(window));
                    }
                    break;
                }
                case WM_NCCALCSIZE:
                {
                    if (!wParam || !is_borderless_resizable(window)) break;
                    remove_frame_from_client_area(hwnd, &reinterpret_cast<NCCALCSIZE_PARAMS *>(lParam)->rgrc[0]);
                    return 0;
                }
                case WM_NCHITTEST:
                {
                    if (!is_borderless_resizable(window)) break;
                    return hit_test_borderless_resize(hwnd, window, lParam);
                }
                case WM_ERASEBKGND:
                {
                    RECT rect{};
                    if (GetClientRect(hwnd, &rect)) fill_window_background(window, reinterpret_cast<HDC>(wParam), rect);
                    return TRUE;
                }

                case WM_PAINT:
                {
                    PAINTSTRUCT ps{};
                    HDC hdc = BeginPaint(hwnd, &ps);
                    fill_window_background(window, hdc, ps.rcPaint);
                    EndPaint(hwnd, &ps);
                    return 0;
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

                    // Keep receiving button-release even when pointer leaves the window.
                    // Do not interfere with hidden-cursor capture mode.
                    if (!(window->state_flags & WindowStateFlagBits::cursor_hidden))
                    {
                        if (action == io::KeyPressState::press)
                        {
                            if (GetCapture() != hwnd) SetCapture(hwnd);
                        }
                        else
                        {
                            const bool any_button_down =
                                (wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) != 0;
                            if (!any_button_down && GetCapture() == hwnd) ReleaseCapture();
                        }
                    }

                    acul::events::dispatch_event_group<awin::MouseClickEvent>(events.mouse_click, window->owner, button,
                                                                              action);
                    break;
                }
                case WM_WINDOWPOSCHANGED:
                {
                    auto *wp = reinterpret_cast<WINDOWPOS *>(lParam);
                    if (window && (wp->flags & 0x8000) && IsIconic(window->hwnd)) on_focus_kill(window);
                    if (window && (!(wp->flags & SWP_NOMOVE) || !(wp->flags & SWP_NOSIZE)))
                        update_window_monitor_if_needed(window);
                    break;
                }
                case WM_SETFOCUS:
                {
                    set_window_state_flag(window->state_flags, WindowStateFlagBits::focused, true);
                    set_window_state_flag(window->state_flags, WindowStateFlagBits::accepts_surface_update, true);
                    acul::events::dispatch_event_group<FocusEvent>(events.focus, window->owner, true);
                    const RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_INPUTSINK, hwnd};
                    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                        AWIN_LOG_ERROR("[Win32] Failed to register raw input device. Error code: %lu", GetLastError());
                    else window->raw_input = true;
                    break;
                }
                case WM_KILLFOCUS:
                    on_focus_kill(window);
                    break;
                case WM_CHAR:
                case WM_SYSCHAR:
                {
                    if (IS_HIGH_SURROGATE(wParam)) window->high_surrogate = wParam;
                    else if (IS_LOW_SURROGATE(wParam))
                    {
                        if (window->high_surrogate)
                        {
                            u32 codepoint = (((window->high_surrogate - 0xD800) << 10) | (wParam - 0xDC00)) + 0x10000;
                            window->high_surrogate = 0;
                            acul::events::dispatch_event_group<CharInputEvent>(events.char_input, window->owner,
                                                                               codepoint);
                        }
                    }
                    else acul::events::dispatch_event_group<CharInputEvent>(events.char_input, window->owner, wParam);

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
                    acul::events::dispatch_event_group<CharInputEvent>(events.char_input, window->owner, wParam);
                    return 0;
                }
                case WM_SYSCOMMAND:
                {
                    switch (wParam & 0xfff0)
                    {
                        case SC_SCREENSAVE:
                        case SC_MONITORPOWER:
                        {
                            if (window->flags & WindowFlagBits::fullscreen) return 0;
                            else break;
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
                            else key = (HIWORD(lParam) & KF_EXTENDED) ? io::Key::rshift : io::Key::lshift;
                            break;
                        }
                        case VK_CONTROL:
                        {
                            if (HIWORD(lParam) & KF_EXTENDED) key = io::Key::rcontrol;
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
                        acul::events::dispatch_event_group<MouseEnterEvent>(events.mouse_enter, window->owner, true);
                    }
                    acul::events::dispatch_event_group<PosEvent>(
                        events.mouse_move, event_id::mouse_move, window->owner,
                        acul::point2D(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                    return 0;
                }
                case WM_MOUSELEAVE:
                    window->cursor_tracked = false;
                    acul::events::dispatch_event_group<MouseEnterEvent>(events.mouse_enter, window->owner, false);
                    return 0;
                case WM_MOUSEWHEEL:
                    acul::events::dispatch_event_group<ScrollEvent>(events.scroll, window->owner, 0,
                                                                    (SHORT)HIWORD(wParam) / (f64)WHEEL_DELTA);
                    return 0;
                case WM_MOUSEHWHEEL:
                {
                    // This message is only sent on Windows Vista and later
                    // NOTE: The X-axis is inverted for consistency with macOS and X11
                    acul::events::dispatch_event_group<ScrollEvent>(events.scroll, window->owner,
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
                            else window->flags &= ~WindowFlagBits::minimized;
                            acul::events::dispatch_event_group<StateEvent>(events.minimize, event_id::minimize,
                                                                           window->owner, want_min);
                        }
                        if ((window->flags & WindowFlagBits::maximized) != want_max)
                        {
                            if (want_max) window->flags |= WindowFlagBits::maximized;
                            else window->flags &= ~WindowFlagBits::maximized;
                            acul::events::dispatch_event_group<StateEvent>(events.maximize, event_id::maximize,
                                                                           window->owner, want_max);
                        }
                    }
                    if (dimenstions != window->dimenstions)
                    {
                        window->dimenstions = dimenstions;
                        ResizeFlags resize_flags{window->resize_flags};
                        if (resize_flags & ResizeFlagBits::repeat_begin)
                        {
                            resize_flags |= ResizeFlagBits::repeat;
                            set_window_state_flag(window->state_flags, WindowStateFlagBits::active_resizing, true);
                        }
                        acul::events::dispatch_event_group<ResizeEvent>(events.resize, window->owner, dimenstions,
                                                                        resize_flags);
                        window->resize_flags &= ~ResizeFlagBits::repeat_begin;
                        update_window_monitor_if_needed(window);
                    }
                    return 0;
                }
                case WM_ENTERSIZEMOVE:
                    if (window)
                    {
                        window->resize_flags |= ResizeFlagBits::repeat | ResizeFlagBits::repeat_begin;
                        set_window_state_flag(window->state_flags, WindowStateFlagBits::accepts_surface_update, false);
                    }
                    break;
                case WM_EXITSIZEMOVE:
                    if (window)
                    {
                        acul::events::dispatch_event_group<ResizeEvent>(
                            events.resize, window->owner, window->dimenstions,
                            ResizeFlagBits::repeat | ResizeFlagBits::repeat_end);
                        window->resize_flags = ResizeFlagBits::none;
                        set_window_state_flag(window->state_flags, WindowStateFlagBits::active_resizing, false);
                        set_window_state_flag(window->state_flags, WindowStateFlagBits::accepts_surface_update,
                                              window->state_flags & WindowStateFlagBits::focused);
                    }
                    break;
                case WM_MOVE:
                    update_window_monitor_if_needed(window);
                    acul::events::dispatch_event_group<PosEvent>(
                        events.move, event_id::move, window->owner,
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
                    ctx.dpi = LOWORD(wParam);
                    const float xscale = LOWORD(wParam) / 96.0f;
                    const float yscale = HIWORD(wParam) / 96.0f;
                    acul::events::dispatch_event_group<DpiChangedEvent>(events.dpi_changed, window->owner, xscale,
                                                                        yscale);
                    break;
                }
                case WM_DISPLAYCHANGE:
                {
                    poll_monitors(g_env->monitors);
                    update_primary_screen_cache();
                    refresh_all_window_monitors();
                    break;
                }
                case WM_DEVICECHANGE:
                {
                    if (wParam == DBT_DEVNODES_CHANGED)
                    {
                        poll_monitors(g_env->monitors);
                        update_primary_screen_cache();
                        refresh_all_window_monitors();
                    }
                    break;
                }
                case WM_SETCURSOR:
                {
                    if (LOWORD(lParam) == HTCLIENT)
                    {
                        if (window->cursor->valid()) window->cursor->assign(nullptr);
                        else SetCursor(LoadCursor(NULL, IDC_ARROW));
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
                        acul::events::dispatch_event_group<PosEvent>(events.mouse_move_delta,
                                                                     event_id::mouse_move_delta, window->owner, delta);
                    }
                    return 0;
                }
                case WM_CLOSE:
                    set_window_state_flag(window->state_flags, WindowStateFlagBits::ready_to_close, true);
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
            if (ctx.com_initialized)
            {
                CoUninitialize();
                ctx.com_initialized = false;
            }
            if (ctx.instance)
            {
                if (ctx.win32_class.hIcon) DestroyIcon(ctx.win32_class.hIcon);
                UnregisterClassW(ctx.win32_class.lpszClassName, ctx.instance);
            }
        }

        void init_timer() { QueryPerformanceFrequency((LARGE_INTEGER *)&g_env->timer.frequency); }

        u64 get_time_value()
        {
            u64 value;
            QueryPerformanceCounter((LARGE_INTEGER *)&value);
            return value;
        }

        u64 get_time_frequency() { return g_env->timer.frequency; }

        bool init_platform()
        {
            ctx.platform_flags = g_env->platform_flags;
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
            ctx.dpi = 96u;
            if (!ctx.win32_class.hIcon)
            {
                AWIN_LOG_WARN("[Win32] Failed to load window icon");
                ctx.win32_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            }
            if (!RegisterClassExW(&ctx.win32_class)) return false;
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            if (FAILED(hr)) return false;
            ctx.com_initialized = true;
            if (!poll_monitors(g_env->monitors)) AWIN_LOG_WARN("[Win32] Failed to poll monitors during init");
            return true;
        }
    } // namespace platform

    Window::Window(const acul::string &title, i32 width, i32 height, WindowFlags flags)
        : _data(acul::alloc<platform::Win32WindowData>())
    {
        auto *wd = (platform::Win32WindowData *)_data;
        wd->owner = this;
        wd->title = acul::utf8_to_utf16(title);
        wd->dimenstions = {width == -1 ? CW_USEDEFAULT : width, height == -1 ? CW_USEDEFAULT : height};
        wd->flags = flags;
        ColorHint next_window_background{};
        const bool has_next_window_background = platform::consume_next_window_hints(next_window_background);

        wd->style = platform::get_window_style(flags);
        wd->ex_style = WS_EX_APPWINDOW;
        wd->hwnd = nullptr;
        wd->cursor = &platform::g_env->default_cursor;
        if (has_next_window_background && next_window_background.enabled)
        {
            wd->has_background_hint = true;
            wd->background_color = RGB(next_window_background.r, next_window_background.g, next_window_background.b);
        }
        const auto window_dimensions =
            platform::client_to_window_dimensions(wd->dimenstions.x, wd->dimenstions.y, wd->style, wd->ex_style, flags);
        wd->hwnd = CreateWindowExW(wd->ex_style, platform::ctx.win32_class.lpszClassName, (LPCWSTR)wd->title.c_str(),
                                   wd->style & ~WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, window_dimensions.x,
                                   window_dimensions.y, nullptr, nullptr, platform::ctx.instance, (LPVOID)wd);

        if (!wd->hwnd) throw acul::runtime_error("Failed to create window");
        if (!(flags & WindowFlagBits::hidden))
        {
            if (flags & WindowFlagBits::minimized) ShowWindow(wd->hwnd, SW_MINIMIZE);
            else if (flags & WindowFlagBits::maximized) ShowWindow(wd->hwnd, SW_MAXIMIZE);
            else ShowWindow(wd->hwnd, SW_SHOWNORMAL);
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
    }

    void Window::show_window()
    {
        if (!hidden()) return;
        auto *wd = (platform::Win32WindowData *)_data;
        WINDOWPLACEMENT placement = {sizeof(WINDOWPLACEMENT)};
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
        if (!(wd->state_flags & WindowStateFlagBits::cursor_hidden)) return;
        cursor_position(wd->saved_cursor_pos);
        ReleaseCapture();
        ShowCursor(TRUE);
        set_window_state_flag(wd->state_flags, WindowStateFlagBits::cursor_hidden, false);
    }

    void Window::hide_cursor()
    {
        auto *wd = (platform::Win32WindowData *)_data;
        if (wd->state_flags & WindowStateFlagBits::cursor_hidden) return;
        wd->saved_cursor_pos = cursor_position();
        SetCapture(wd->hwnd);
        ShowCursor(FALSE);
        ClipCursor(NULL);
        set_window_state_flag(wd->state_flags, WindowStateFlagBits::cursor_hidden, true);
    }

    acul::point2D<i32> Window::position() const
    {
        RECT rect;
        auto *wd = (platform::Win32WindowData *)_data;
        if (GetWindowRect(wd->hwnd, &rect)) return {rect.left, rect.top};
        else return {0, 0};
    }

    void Window::position(acul::point2D<i32> position)
    {
        WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
        auto *wd = (platform::Win32WindowData *)_data;
        GetWindowPlacement(wd->hwnd, &wp);

        RECT rect{};
        acul::point2D<i32> dimensions{};
        if (GetWindowRect(wd->hwnd, &rect)) dimensions = {rect.right - rect.left, rect.bottom - rect.top};
        else dimensions = wd->dimenstions;
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

    void set_timeout(f64 timeout) { platform::g_env->timeout = timeout; }

    void wait_events()
    {
        WaitMessage();
        poll_events();
    }

    void poll_events()
    {
        MSG msg = {};

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    void wait_events_timeout()
    {
        if (platform::g_env->timeout > AWIN_TIMEOUT_INF)
        {
            MsgWaitForMultipleObjects(0, NULL, FALSE, platform::g_env->timeout * 1e3, QS_ALLINPUT);
            platform::g_env->timeout = AWIN_TIMEOUT_INF;
        }
        else WaitMessage();
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

    acul::point2D<i32> get_window_size_origin(const Window &window)
    {
        auto *wd = reinterpret_cast<platform::Win32WindowData *>(get_window_data(window));
        if (platform::is_borderless_resizable(wd))
        {
            RECT rect{};
            if (GetWindowRect(wd->hwnd, &rect)) return {rect.right - rect.left, rect.bottom - rect.top};
        }
        return get_window_size(window);
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
        platform::g_env->clipboard_data = acul::utf16_to_utf8(buffer);
        GlobalUnlock(object);
        CloseClipboard();
        return platform::g_env->clipboard_data;
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

    WindowFlags get_window_flags(const Window &window)
    {
        auto *wd = (platform::Win32WindowData *)get_window_data(window);
        return wd->flags;
    }
} // namespace awin
