#include <acul/string/string.hpp>
#include <awin/awin.hpp>
#include <awin/native_access.hpp>
#include <dbt.h>
#include <shlobj.h>
#include <windef.h>
#include <windowsx.h>
#if defined(AWIN_WIN32_APP_SDK_ENABLED)
    #include <winrt/Microsoft.UI.Interop.h>
    #include <winrt/Microsoft.UI.Windowing.h>
    #include <winrt/Windows.Foundation.Collections.h>
    #include <winrt/Windows.Graphics.h>
    #include <winrt/Windows.UI.h>
    #include <winrt/base.h>
#endif
#if defined(AWIN_WIN32_APP_SDK_BOOTSTRAP_ENABLED)
    #include <MddBootstrap.h>
#endif
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
            bool background_hint_active{false};
            COLORREF background_color{RGB(35, 35, 35)};
#if defined(AWIN_WIN32_APP_SDK_ENABLED)
            bool windows_app_sdk_enabled{false};
            bool extends_content_into_title_bar{false};
            winrt::Microsoft::UI::Windowing::AppWindow app_window{nullptr};
            winrt::Microsoft::UI::Windowing::AppWindowTitleBar title_bar{nullptr};
            native_access::Win32TitleBarMetrics title_bar_metrics{};
#endif
        };

#if defined(AWIN_WIN32_APP_SDK_ENABLED)
        static winrt::Windows::UI::Color to_winrt_color(const ColorHint &color)
        {
            winrt::Windows::UI::Color result{};
            result.A = color.a;
            result.R = color.r;
            result.G = color.g;
            result.B = color.b;
            return result;
        }

        static bool update_title_bar_metrics(Win32WindowData *window)
        {
            if (!window || !window->title_bar) return false;
            try
            {
                window->title_bar_metrics.left_inset = window->title_bar.LeftInset();
                window->title_bar_metrics.right_inset = window->title_bar.RightInset();
                window->title_bar_metrics.height = window->title_bar.Height();
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static bool initialize_app_sdk_window(Win32WindowData *window)
        {
            if (!window || !ctx.windows_app_sdk_enabled || !window->hwnd) return false;

            try
            {
                const auto id = winrt::Microsoft::UI::GetWindowIdFromWindow(window->hwnd);
                window->app_window = winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(id);
                window->title_bar = window->app_window ? window->app_window.TitleBar() : nullptr;
                window->windows_app_sdk_enabled = window->app_window != nullptr;
                if (window->title_bar) update_title_bar_metrics(window);
                return window->windows_app_sdk_enabled;
            }
            catch (...)
            {
                window->app_window = nullptr;
                window->title_bar = nullptr;
                window->windows_app_sdk_enabled = false;
                return false;
            }
        }

        static void apply_optional_color(const ColorHint &value, auto setter)
        {
            if (value.enabled) setter(to_winrt_color(value));
        }

        static bool apply_title_bar_config(Win32WindowData *window, const WindowTitleBarHints &config)
        {
            if (!window || !config.enabled || !window->windows_app_sdk_enabled || !window->title_bar) return false;
            if (!winrt::Microsoft::UI::Windowing::AppWindowTitleBar::IsCustomizationSupported()) return false;

            try
            {
                window->extends_content_into_title_bar = config.extends_content_into_title_bar;
                window->title_bar.ExtendsContentIntoTitleBar(config.extends_content_into_title_bar);

                apply_optional_color(config.background, [&](const auto &c) { window->title_bar.BackgroundColor(c); });
                apply_optional_color(config.foreground, [&](const auto &c) { window->title_bar.ForegroundColor(c); });
                apply_optional_color(config.inactive_background,
                                     [&](const auto &c) { window->title_bar.InactiveBackgroundColor(c); });
                apply_optional_color(config.inactive_foreground,
                                     [&](const auto &c) { window->title_bar.InactiveForegroundColor(c); });
                apply_optional_color(config.buttons.background,
                                     [&](const auto &c) { window->title_bar.ButtonBackgroundColor(c); });
                apply_optional_color(config.buttons.foreground,
                                     [&](const auto &c) { window->title_bar.ButtonForegroundColor(c); });
                apply_optional_color(config.buttons.hover_background,
                                     [&](const auto &c) { window->title_bar.ButtonHoverBackgroundColor(c); });
                apply_optional_color(config.buttons.hover_foreground,
                                     [&](const auto &c) { window->title_bar.ButtonHoverForegroundColor(c); });
                apply_optional_color(config.buttons.pressed_background,
                                     [&](const auto &c) { window->title_bar.ButtonPressedBackgroundColor(c); });
                apply_optional_color(config.buttons.pressed_foreground,
                                     [&](const auto &c) { window->title_bar.ButtonPressedForegroundColor(c); });
                apply_optional_color(config.buttons.inactive_background,
                                     [&](const auto &c) { window->title_bar.ButtonInactiveBackgroundColor(c); });
                apply_optional_color(config.buttons.inactive_foreground,
                                     [&](const auto &c) { window->title_bar.ButtonInactiveForegroundColor(c); });

                update_title_bar_metrics(window);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
#endif

        DWORD get_window_style(WindowFlags flags)
        {
            DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            if (flags & WindowFlagBits::fullscreen) style |= WS_POPUP;
            if (flags & WindowFlagBits::decorated) style |= WS_SYSMENU;
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

        void on_focus_kill(Win32WindowData *wd)
        {
            if (!wd) return;
            wd->focused = false;
            acul::events::dispatch_event_group<FocusEvent>(g_env->events.focus, wd->owner, false);
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
                    break;
                }
                case WM_NCCREATE:
                    EnableNonClientDpiScaling(hwnd);
                    break;
                case WM_ERASEBKGND:
                {
                    if (!window || !window->has_background_hint || !window->background_hint_active) return TRUE;
                    HDC hdc = (HDC)wParam;
                    HBRUSH hBrush = CreateSolidBrush(window->background_color);
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    FillRect(hdc, &rect, hBrush);
                    DeleteObject(hBrush);
                    return TRUE;
                }
                case WM_PAINT:
                    if (window && window->background_hint_active) window->background_hint_active = false;
                    break;

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
                    if (!window->is_cursor_hidden)
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
                    if ((wp->flags & 0x8000) && IsIconic(window->hwnd)) on_focus_kill(window);
                    break;
                }
                case WM_SETFOCUS:
                {
                    window->focused = true;
                    acul::events::dispatch_event_group<FocusEvent>(events.focus, window->owner, true);
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
                            acul::events::dispatch_event_group<CharInputEvent>(events.char_input, window->owner,
                                                                               codepoint);
                        }
                    }
                    else
                        acul::events::dispatch_event_group<CharInputEvent>(events.char_input, window->owner, wParam);

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
#if defined(AWIN_WIN32_APP_SDK_ENABLED)
                    if (window && window->windows_app_sdk_enabled) update_title_bar_metrics(window);
#endif
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
                            acul::events::dispatch_event_group<StateEvent>(events.minimize, event_id::minimize,
                                                                           window->owner, want_min);
                        }
                        if ((window->flags & WindowFlagBits::maximized) != want_max)
                        {
                            if (want_max)
                                window->flags |= WindowFlagBits::maximized;
                            else
                                window->flags &= ~WindowFlagBits::maximized;
                            acul::events::dispatch_event_group<StateEvent>(events.maximize, event_id::maximize,
                                                                           window->owner, want_max);
                        }
                    }
                    if (dimenstions != window->dimenstions)
                    {
                        window->dimenstions = dimenstions;
                        acul::events::dispatch_event_group<PosEvent>(events.resize, event_id::resize, window->owner,
                                                                     dimenstions);
                    }
                    return 0;
                }
                case WM_MOVE:
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
#if defined(AWIN_WIN32_APP_SDK_ENABLED)
                    if (window && window->windows_app_sdk_enabled) update_title_bar_metrics(window);
#endif
                    acul::events::dispatch_event_group<DpiChangedEvent>(events.dpi_changed, window->owner, xscale,
                                                                        yscale);
                    break;
                }
                case WM_DISPLAYCHANGE:
                {
                    poll_monitors(g_env->monitors);
                    update_primary_screen_cache();
                    break;
                }
                case WM_DEVICECHANGE:
                {
                    if (wParam == DBT_DEVNODES_CHANGED)
                    {
                        poll_monitors(g_env->monitors);
                        update_primary_screen_cache();
                    }
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
                        acul::events::dispatch_event_group<PosEvent>(events.mouse_move_delta,
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
#if defined(AWIN_WIN32_APP_SDK_BOOTSTRAP_ENABLED)
            if (ctx.windows_app_sdk_bootstrapped)
            {
                MddBootstrapShutdown();
                ctx.windows_app_sdk_bootstrapped = false;
            }
#endif
            if (ctx.com_initialized)
            {
#if defined(AWIN_WIN32_APP_SDK_ENABLED)
                if (ctx.windows_app_sdk_enabled)
                    winrt::uninit_apartment();
                else
#endif
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
            if ((ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK_BOOTSTRAP) &&
                !(ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK))
            {
                AWIN_LOG_ERROR("[Win32] Bootstrap requires AWIN_PLATFORM_WIN32_APP_SDK");
                return false;
            }
#if !defined(AWIN_WIN32_APP_SDK_ENABLED)
            if (ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK)
            {
                AWIN_LOG_ERROR("[Win32] Windows App SDK was requested but awin was built without App SDK support");
                return false;
            }
#endif
#if !defined(AWIN_WIN32_APP_SDK_BOOTSTRAP_ENABLED)
            if (ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK_BOOTSTRAP)
            {
                AWIN_LOG_ERROR(
                    "[Win32] Windows App SDK bootstrap was requested but awin was built without bootstrap support");
                return false;
            }
#endif
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
#if defined(AWIN_WIN32_APP_SDK_ENABLED)
            if (ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK)
            {
                try
                {
                    winrt::init_apartment(winrt::apartment_type::single_threaded);
                    ctx.com_initialized = true;
                    ctx.windows_app_sdk_enabled = true;
                }
                catch (...)
                {
                    AWIN_LOG_ERROR("[Win32] Failed to initialize WinRT apartment");
                    return false;
                }
            }
            else
#endif
            {
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                if (FAILED(hr)) return false;
                ctx.com_initialized = true;
            }
#if defined(AWIN_WIN32_APP_SDK_BOOTSTRAP_ENABLED)
            if (ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK_BOOTSTRAP)
            {
                constexpr UINT32 release_major_minor = (static_cast<UINT32>(AWIN_WIN32_APP_SDK_VERSION_MAJOR) << 16) |
                                                       static_cast<UINT32>(AWIN_WIN32_APP_SDK_VERSION_MINOR);
                const HRESULT bootstrap_hr = MddBootstrapInitialize(release_major_minor, L"", {});
                if (FAILED(bootstrap_hr))
                {
                    AWIN_LOG_ERROR("[Win32] MddBootstrapInitialize failed: 0x%08lx", bootstrap_hr);
                    if (ctx.com_initialized)
                    {
    #if defined(AWIN_WIN32_APP_SDK_ENABLED)
                        if (ctx.windows_app_sdk_enabled)
                            winrt::uninit_apartment();
                        else
    #endif
                            CoUninitialize();
                        ctx.com_initialized = false;
                    }
                    return false;
                }
                ctx.windows_app_sdk_bootstrapped = true;
            }
#endif
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
        WindowHints next_window_hints{};
        const bool has_next_window_hints = platform::consume_next_window_hints(next_window_hints);

        wd->style = platform::get_window_style(flags);
        wd->ex_style = WS_EX_APPWINDOW;
        wd->hwnd = nullptr;
        wd->cursor = &platform::g_env->default_cursor;
        if (has_next_window_hints && next_window_hints.background.enabled)
        {
            wd->has_background_hint = true;
            wd->background_hint_active = true;
            wd->background_color =
                RGB(next_window_hints.background.r, next_window_hints.background.g, next_window_hints.background.b);
        }
        wd->hwnd = CreateWindowExW(wd->ex_style, platform::ctx.win32_class.lpszClassName, (LPCWSTR)wd->title.c_str(),
                                   wd->style & ~WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, wd->dimenstions.x,
                                   wd->dimenstions.y, nullptr, nullptr, platform::ctx.instance, (LPVOID)wd);

        if (!wd->hwnd) throw acul::runtime_error("Failed to create window");
#if defined(AWIN_WIN32_APP_SDK_ENABLED)
        if (platform::ctx.windows_app_sdk_enabled && !platform::initialize_app_sdk_window(wd))
            throw acul::runtime_error("Failed to initialize Windows App SDK window state");
        if (has_next_window_hints && next_window_hints.title_bar.enabled)
            platform::apply_title_bar_config(wd, next_window_hints.title_bar);
#endif
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
    }

    void Window::show_window()
    {
        if (!hidden()) return;
        auto *wd = (platform::Win32WindowData *)_data;
        if (wd->has_background_hint) wd->background_hint_active = true;
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

        acul::point2D<i32> dimensions = get_window_size(*this);
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

#if defined(AWIN_WIN32_APP_SDK_ENABLED)
    bool native_access::set_win32_title_bar_config(Window &window, const WindowTitleBarHints &config)
    {
        if (!(platform::ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK)) return false;
        auto *wd = static_cast<platform::Win32WindowData *>(get_window_data(window));
        return platform::apply_title_bar_config(wd, config);
    }

    bool native_access::get_win32_title_bar_metrics(const Window &window, Win32TitleBarMetrics &metrics)
    {
        if (!(platform::ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK)) return false;
        auto *wd = static_cast<platform::Win32WindowData *>(get_window_data(window));
        if (!platform::update_title_bar_metrics(wd)) return false;
        metrics = wd->title_bar_metrics;
        return true;
    }

    bool native_access::set_win32_title_bar_drag_rects(const Window &window,
                                                       const acul::vector<Win32TitleBarDragRect> &rects)
    {
        if (!(platform::ctx.platform_flags & AWIN_PLATFORM_WIN32_APP_SDK)) return false;
        auto *wd = static_cast<platform::Win32WindowData *>(get_window_data(window));
        if (!wd || !wd->windows_app_sdk_enabled || !wd->title_bar || !wd->extends_content_into_title_bar) return false;

        try
        {
            acul::vector<winrt::Windows::Graphics::RectInt32> native_rects;
            native_rects.resize(rects.size());
            for (size_t i = 0; i < rects.size(); ++i)
            {
                native_rects[i] = {rects[i].x, rects[i].y, rects[i].width, rects[i].height};
            }
            wd->title_bar.SetDragRectangles(
                winrt::array_view<const winrt::Windows::Graphics::RectInt32>(native_rects.data(), native_rects.size()));
            platform::update_title_bar_metrics(wd);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
#endif

} // namespace awin
