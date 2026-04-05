#pragma once

#include <X11/X.h>
#include "awin.hpp"
#ifdef _WIN32
    #include <windows.h>
#else
struct wl_surface;
#endif

namespace awin
{
    namespace native_access
    {
#ifdef _WIN32
    #if defined(AWIN_WIN32_APP_SDK_ENABLED)
        struct Win32TitleBarMetrics
        {
            i32 left_inset{0};
            i32 right_inset{0};
            i32 height{0};
        };

        struct Win32TitleBarDragRect
        {
            i32 x{0};
            i32 y{0};
            i32 width{0};
            i32 height{0};
        };

        APPLIB_API bool set_win32_title_bar_config(Window &window, const WindowTitleBarHints &config);
        APPLIB_API bool get_win32_title_bar_metrics(const Window &window, Win32TitleBarMetrics &metrics);
        APPLIB_API bool set_win32_title_bar_drag_rects(const Window &window,
                                                       const acul::vector<Win32TitleBarDragRect> &rects);
    #endif
        APPLIB_API HWND get_hwnd(const Window &window);
#else
        APPLIB_API int get_backend_type();
        APPLIB_API ::Window get_x11_window_handle(const Window &window);
        APPLIB_API wl_surface *get_wayland_surface(const Window &window);
    #if defined(AWIN_TEST_BUILD) || defined(PROCESS_UNITTEST)
        APPLIB_API void enable_wayland_surface_placeholder();
    #endif
#endif
    }; // namespace native_access
} // namespace awin
