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
        AWIN_EXPORT HWND get_hwnd(const Window &window);
#else
        AWIN_EXPORT int get_backend_type();
        AWIN_EXPORT ::Window get_x11_window_handle(const Window &window);
        AWIN_EXPORT wl_surface *get_wayland_surface(const Window &window);
    #if defined(AWIN_TEST_BUILD) || defined(PROCESS_UNITTEST)
        AWIN_EXPORT void enable_wayland_surface_placeholder();
    #endif
#endif
    }; // namespace native_access
} // namespace awin
