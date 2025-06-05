#pragma once

#include <X11/Xlib.h>
#include <acul/pair.hpp>
#include <acul/string/string.hpp>
#include <poll.h>
#include "platform.hpp"

namespace awin
{
    namespace platform
    {
        struct WindowData;

        namespace x11
        {
            struct X11WindowData
            {
                XID window, parent;
                XIC ic;
                Colormap colormap;
                acul::point2D<int> pos, size;
                // The time of the last KeyPress event per keycode, for discarding
                // duplicate key events generated for some keys by ibus
                Time key_press_times[256] = {0};
            };

            // Push contents of our selection to clipboard manager
            void push_selection_to_manager_x11();

            void create_input_context(X11WindowData *window);

            bool create_window(WindowData *window_data, const acul::string &title, i32 width, i32 height,
                               CreationFlags flags);

            inline void get_window_pos(const X11WindowData &window_data, acul::point2D<i32> &pos)
            {
                Window dummy;
                ctx.loader.XTranslateCoordinates(ctx.display, window_data.window, ctx.root, 0, 0, &pos.x, &pos.y,
                                                 &dummy);
            }

            inline void get_window_size(const X11WindowData &window_data, acul::point2D<i32> &size)
            {
                XWindowAttributes attribs;
                ctx.loader.XGetWindowAttributes(ctx.display, window_data.window, &attribs);
            }

            inline bool is_window_visible_x11(const X11WindowData &window_data)
            {
                XWindowAttributes wa;
                ctx.loader.XGetWindowAttributes(ctx.display, window_data.window, &wa);
                return wa.map_state == IsViewable;
            }

            bool wait_for_x11_event(f64 *timeout);

            inline bool wait_for_visibility_notify(const X11WindowData &window_data)
            {
                XEvent dummy;
                f64 timeout = 0.1;
                while (!ctx.loader.XCheckTypedWindowEvent(ctx.display, window_data.window, VisibilityNotify, &dummy))
                    if (!wait_for_x11_event(&timeout)) return false;
                return true;
            }

            inline void show_window(X11WindowData &window_data)
            {
                if (is_window_visible_x11(window_data)) return;
                ctx.loader.XMapWindow(ctx.display, window_data.window);
                wait_for_visibility_notify(window_data);
            }

            void poll_events();
        } // namespace x11
    } // namespace platform
} // namespace awin