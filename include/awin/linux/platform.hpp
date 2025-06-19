#pragma once

#include <acul/pair.hpp>
#include <acul/string/string.hpp>
#include <acul/vector.hpp>
#include <sys/poll.h>
#include "../types.hpp"

#define WINDOW_BACKEND_UNKNOWN -1
#define WINDOW_BACKEND_X11     0
#define WINDOW_BACKEND_WAYLAND 1

namespace awin
{
    class Window;
    namespace platform
    {
        struct WindowData;
        struct LinuxWindowImpl
        {
        };

        struct LinuxWindowData
        {
            LinuxWindowImpl *impl;
            acul::point2D<int> pos;
            bool raw_input{false};
        };

        using platform_data_t = LinuxWindowData;

        struct LinuxAccessConnect
        {
            static APPLIB_API LinuxWindowImpl *get_impl(const Window &window);
            static APPLIB_API int get_backend_type();
        };

        using native_access = LinuxAccessConnect;

        bool poll_posix(struct pollfd *fds, nfds_t count, f64 *timeout);

        struct LinuxWindowCaller
        {
            bool (*create_window)(platform::WindowData *, const acul::string &, i32, i32, WindowFlags);
            void (*set_window_icon)(platform::LinuxWindowImpl *, const acul::vector<Image> &);
            void (*show_window)(platform::LinuxWindowImpl *);
            void (*hide_window)(platform::LinuxWindowImpl *);
            acul::string (*get_window_title)(platform::LinuxWindowImpl *);
            void (*set_window_title)(platform::LinuxWindowImpl *, const acul::string &);
            void (*enable_fullscreen)(platform::LinuxWindowImpl *);
            void (*disable_fullscreen)(platform::LinuxWindowImpl *);
            acul::point2D<i32> (*get_cursor_position)(platform::LinuxWindowImpl *);
            void (*set_cursor_position)(platform::LinuxWindowImpl *, acul::point2D<i32>);
            void (*hide_cursor)(WindowData *);
            void (*show_cursor)(WindowData *);
            acul::point2D<i32> (*get_window_position)(LinuxWindowImpl *);
            void (*set_window_position)(WindowData *, acul::point2D<i32>);
            void (*destroy)(platform::LinuxWindowImpl *);
        };

        struct LinuxPlatformCaller
        {
            bool (*init_platform)();
            void (*destroy_platform)();
            LinuxWindowImpl *(*alloc_window_impl)();
            void (*poll_events)();
            void (*wait_events)();
            void (*wait_events_timeout)();
        };
    } // namespace platform

    APPLIB_API void set_window_icon(Window &window, const acul::vector<Image> &images);
} // namespace awin