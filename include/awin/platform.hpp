#pragma once

#include <acul/event.hpp>
#include <acul/pair.hpp>
#include <acul/string/string.hpp>
#include "types.hpp"
#ifdef _WIN32
    #include <windows.h>
#else
    #define WINDOW_BACKEND_UNKNOWN -1
    #define WINDOW_BACKEND_X11     0
    #define WINDOW_BACKEND_WAYLAND 1
#endif

#define WINDOW_TIMEOUT_INF -1

namespace awin
{
    class Window;
    namespace platform
    {
        // The window environment configuration for window management and interactions within the windowing system.
        extern APPLIB_API struct WindowEnvironment
        {
            acul::string clipboard_data; // Clipboard data storage.
            struct Timer
            {
#ifndef _WIN32
                clockid_t clock_id;
#endif
                u64 offset;                   // Time offset
                u64 frequency;                // Timer frequency
            } timer;                          // Timer information for time tracking.
            f64 timeout = WINDOW_TIMEOUT_INF; // Global timeout for waking up the main loop.
            acul::events::dispatcher *ed = nullptr;
            Cursor default_cursor;
        } env;

#ifdef _WIN32
        struct Win32PlatformData
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

        using platform_data_t = Win32PlatformData;

        struct Win32AccessConnect
        {
            static APPLIB_API HWND get_hwnd(const Window &window);

            static APPLIB_API acul::point2D<i32> get_full_client_size(const Window &window);
        };

        using native_access = Win32AccessConnect;
#else
        struct LinuxWindowData
        {
            acul::point2D<int> window_pos;
        };

        using platform_data_t = LinuxWindowData;

        struct LinuxAccessConnect
        {
            static APPLIB_API LinuxWindowData *get_window_data(const Window &window);
            static APPLIB_API int get_backend_type();
        };

        using native_access = LinuxAccessConnect;
#endif

        struct WindowData
        {
            Window *owner;
            acul::point2D<i32> dimenstions;
            WindowFlags flags;
            bool is_cursor_hidden{false};
            bool focused{false};
            bool ready_to_close = false;
            acul::point2D<i32> resize_limit{0, 0};
            io::KeyPressState keys[io::Key::Last + 1];
            Cursor *cursor{NULL};
#ifdef _WIN32
            platform_data_t backend{};
#else
            platform_data_t *backend{NULL};
            ~WindowData() { acul::release(backend); }
#endif
        };
    } // namespace platform

    inline void update_timeout(f64 timeout) { platform::env.timeout = std::max(platform::env.timeout, timeout); }
} // namespace awin