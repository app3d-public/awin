#pragma once

#include <awin/platform.hpp>
#include <awin/types.hpp>
#include <sys/poll.h>

namespace awin
{
    namespace platform
    {
        bool poll_posix(struct pollfd *fds, nfds_t count, f64 *timeout);

        struct LinuxWindowCaller
        {
            bool (*create_window)(WindowData *, const acul::string &, i32, i32, WindowFlags);
            void (*set_window_icon)(LinuxWindowData *, const acul::vector<Image> &);
            void (*show_window)(WindowData *);
            void (*hide_window)(LinuxWindowData *);
            acul::string (*get_window_title)(LinuxWindowData *);
            void (*set_window_title)(LinuxWindowData *, const acul::string &);
            void (*enable_fullscreen)(LinuxWindowData *);
            void (*disable_fullscreen)(LinuxWindowData *);
            acul::point2D<i32> (*get_cursor_position)(LinuxWindowData *);
            void (*set_cursor_position)(LinuxWindowData *, acul::point2D<i32>);
            void (*hide_cursor)(WindowData *);
            void (*show_cursor)(Window *, WindowData *);
            acul::point2D<i32> (*get_window_position)(LinuxWindowData *);
            void (*set_window_position)(WindowData *, acul::point2D<i32>);
            void (*center_window)(WindowData *);
            void (*update_resize_limit)(WindowData *);
            void (*minimize_window)(LinuxWindowData *);
            void (*maximize_window)(WindowData *);
            void (*destroy)(LinuxWindowData *);
        };

        struct LinuxPlatformCaller
        {
            bool (*init_platform)();
            void (*destroy_platform)();
            LinuxWindowData *(*alloc_window_data)();
            void (*poll_events)();
            void (*wait_events)();
            void (*wait_events_timeout)();
            void (*push_empty_event)();
            f32 (*get_dpi)();
            acul::point2D<i32> (*get_window_size)(LinuxWindowData *);
            acul::string (*get_clipboard_string)();
            void (*set_clipboard_string)(const acul::string &);
            MonitorInfo (*get_primary_monitor_info)();
        };

        struct LinuxCursorCaller
        {
            platform::CursorPlatform *(*create)(Cursor::Type) = NULL;
            void (*assign)(struct LinuxWindowData *, platform::CursorPlatform *) = NULL;
            void (*destroy)(platform::CursorPlatform *) = NULL;
            bool (*valid)(const platform::CursorPlatform *) = NULL;
        };

        void init_call_cdata(LinuxCursorCaller &caller);
    } // namespace platform
} // namespace awin