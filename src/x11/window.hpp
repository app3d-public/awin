#pragma once

#include <X11/Xlib.h>
#include <acul/pair.hpp>
#include <acul/string/string.hpp>
#include <awin/platform.hpp>
#include <poll.h>

namespace awin
{
    namespace platform
    {
        struct WindowData;

        namespace x11
        {
            struct X11WindowData final : LinuxWindowData
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

            void show_window(platform::LinuxWindowData *window_data);
            void hide_window(platform::LinuxWindowData *window_data);

            acul::string get_window_title(platform::LinuxWindowData *window_data);
            void set_window_title(platform::LinuxWindowData *window_data, const acul::string &title);

            void enable_fullscreen(platform::LinuxWindowData *window_data);
            void disable_fullscreen(platform::LinuxWindowData *window_data);

            acul::point2D<i32> get_cursor_position(platform::LinuxWindowData *window_data);
            void set_cursor_position(platform::LinuxWindowData *window_data, acul::point2D<i32> position);

            void hide_cursor(WindowData *window_data);
            void show_cursor(WindowData *window_data);

            acul::point2D<i32> get_window_position(LinuxWindowData *window);
            void set_window_position(WindowData *window, acul::point2D<i32> position);
            void center_window(WindowData *window);
            void update_resize_limit(WindowData *window);

            void minimize_window(LinuxWindowData *window);
            void maximize_window(WindowData *window);

            void destroy(platform::LinuxWindowData *);

            void poll_events();
            void wait_events();
            void wait_events_timeout();
            void push_empty_event();

            f32 get_dpi();
            acul::point2D<i32> get_window_size(LinuxWindowData *window);

            acul::string get_clipboard_string();
            void set_clipboard_string(const acul::string &text);

            void set_window_icon(platform::LinuxWindowData *, const acul::vector<Image> &);
        } // namespace x11
    } // namespace platform
} // namespace awin