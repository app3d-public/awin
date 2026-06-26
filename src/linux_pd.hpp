#pragma once

#include <X11/X.h>
#include <acul/string/string.hpp>
#include <acul/vector.hpp>
#include <awin/types.hpp>
#include <sys/poll.h>

#define AWIN_RESIZE_END_TIMEOUT 0.120
#define AWIN_RESIZE_POLL_TICK   0.016

namespace awin
{
    struct Monitor;
    struct VidMode;

    struct Cursor::Platform
    {
        ~Platform();
    };

    inline Cursor::Platform *get_cursor_pd(Cursor *cursor) { return cursor->_pd; }

    namespace platform
    {
        bool poll_posix(struct pollfd *fds, nfds_t count, f64 *timeout);

        struct LinuxWindowCaller
        {
            bool (*create_window)(WindowData *, const acul::string &, i32, i32, WindowFlags);
            void (*set_window_icon)(WindowData *, const acul::vector<Image> &);
            void (*show_window)(WindowData *);
            void (*hide_window)(WindowData *);
            acul::string (*get_window_title)(WindowData *);
            void (*set_window_title)(WindowData *, const acul::string &);
            void (*enable_fullscreen)(WindowData *);
            void (*disable_fullscreen)(WindowData *);
            acul::point2D<i32> (*get_cursor_position)(WindowData *);
            void (*set_cursor_position)(WindowData *, acul::point2D<i32>);
            void (*hide_cursor)(WindowData *);
            void (*show_cursor)(Window *, WindowData *);
            acul::point2D<i32> (*get_window_position)(WindowData *);
            void (*set_window_position)(WindowData *, acul::point2D<i32>);
            void (*center_window)(WindowData *);
            void (*update_resize_limit)(WindowData *);
            void (*minimize_window)(WindowData *);
            void (*maximize_window)(WindowData *);
            void (*destroy)(WindowData *);
        };

        struct LinuxPlatformCaller
        {
            bool (*init_platform)();
            void (*destroy_platform)();
            bool (*poll_monitors)(acul::vector<Monitor> &);
            bool (*get_monitor_video_modes)(const Monitor *, acul::vector<VidMode> &);
            bool (*get_monitor_video_mode)(const Monitor *, VidMode &);
            WindowData *(*alloc_window_data)();
            void (*poll_events)();
            void (*wait_events)();
            void (*wait_events_timeout)();
            void (*push_empty_event)();
            f32 (*get_dpi)(WindowData *);
            acul::point2D<i32> (*get_window_size)(const Window &);
            acul::string (*get_clipboard_string)();
            void (*set_clipboard_string)(const acul::string &);
        };

        struct LinuxCursorCaller
        {
            Cursor::Platform *(*create)(Cursor::Type) = NULL;
            void (*assign)(Window *, Cursor::Platform *) = NULL;
            void (*destroy)(Cursor::Platform *) = NULL;
            bool (*valid)(const Cursor::Platform *) = NULL;
        };

        void init_call_cdata(LinuxCursorCaller &caller);

        void sync_mods_by_key(io::Key key, io::KeyMode &mods);
    } // namespace platform
} // namespace awin
