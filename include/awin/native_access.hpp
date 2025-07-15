#pragma once

#include <X11/X.h>
#include "window.hpp"
#ifdef _WIN32
    #include <windows.h>
#endif

namespace awin
{
    struct native_access
    {
#ifdef _WIN32
        static APPLIB_API HWND get_hwnd(const Window &window);
        static APPLIB_API acul::point2D<i32> get_full_client_size(const Window &window);
#else   
        static APPLIB_API int get_backend_type();
        static APPLIB_API ::Window get_x11_window_handle(const Window &window);
#endif
    };
} // namespace awin