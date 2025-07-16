#pragma once

#include <X11/X.h>
#include "window.hpp"
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
        APPLIB_API HWND get_hwnd(const Window &window);
        APPLIB_API acul::point2D<i32> get_full_client_size(const Window &window);
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