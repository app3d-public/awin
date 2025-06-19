#pragma once

#include <X11/Xlib.h>
#include <acul/pair.hpp>
#include <acul/string/string.hpp>
#include <awin/linux/platform.hpp>
#include <poll.h>

namespace awin
{
    namespace platform
    {
        struct WindowData;

        namespace x11
        {
            struct X11WindowData final : LinuxWindowImpl
            {
                XID window, parent;
                XIC ic;
                Colormap colormap;
                // The time of the last KeyPress event per keycode, for discarding
                // duplicate key events generated for some keys by ibus
                Time key_press_times[256] = {0};
            };

            // Push contents of our selection to clipboard manager
            void push_selection_to_manager_x11();

            void create_input_context(X11WindowData *window);

            bool create_window(WindowData *window_data, const acul::string &title, i32 width, i32 height,
                               WindowFlags flags);

            bool wait_for_x11_event(f64 *timeout);

            void show_window(platform::LinuxWindowImpl *window_data);
            void hide_window(platform::LinuxWindowImpl *window_data);

            acul::string get_window_title(platform::LinuxWindowImpl *window_data);
            void set_window_title(platform::LinuxWindowImpl *window_data, const acul::string &title);

            void enable_fullscreen(platform::LinuxWindowImpl *window_data);
            void disable_fullscreen(platform::LinuxWindowImpl *window_data);

            acul::point2D<i32> get_cursor_position(platform::LinuxWindowImpl *window_data);
            void set_cursor_position(platform::LinuxWindowImpl *window_data, acul::point2D<i32> position);

            void hide_cursor(WindowData *window_data);
            void show_cursor(WindowData *window_data);

            acul::point2D<i32> get_window_position(LinuxWindowImpl *window);
            void set_window_position(WindowData *window, acul::point2D<i32> position);

            void destroy(platform::LinuxWindowImpl *);
            void poll_events();
            void wait_events();
            void wait_events_timeout();
            void set_window_icon(platform::LinuxWindowImpl *, const acul::vector<Image> &);
        } // namespace x11
    } // namespace platform
} // namespace awin