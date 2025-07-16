#pragma once

#include <acul/string/string.hpp>
#include <awin/types.hpp>
#include "loaders.hpp"
#include "platform.hpp"

namespace awin
{
    namespace platform
    {
        namespace wayland
        {
            struct FallbackEdgeWayland
            {
                wl_surface *surface;
                wl_subsurface *subsurface;
                wp_viewport *viewport;
            };

            struct WaylandWindowData final : WindowData
            {
                wl_surface *surface;
                acul::string title;
                bool hovered, activated;
                i32 buffer_scale;
                acul::point2D<f64> cursor_pos;
                Output *output;
                acul::vector<OutputScale> output_scales;
                bool scale_framebuffer;
                u32 scaling_numerator;
                wp_viewport *scaling_viewport;
                wp_fractional_scale_v1 *fractional_scale;
                zwp_idle_inhibitor_v1 *idle_inhibitor;
                zwp_relative_pointer_v1 *relative_pointer;
                ::libdecor_frame *libdecor_frame;
                struct
                {
                    bool decorations;
                    wl_buffer *buffer;
                    FallbackEdgeWayland top, left, right, bottom;
                    wl_surface *focus;
                } fallback;
                struct
                {
                    xdg_surface *surface;
                    xdg_toplevel *toplevel;
                    zxdg_toplevel_decoration_v1 *decoration;
                    u32 decoration_mode;
                } xdg;
                struct Pending
                {
                    acul::point2D<int> dimensions;
                    WindowFlags flags;
                } pending;
            };

            bool create_window(WindowData *window_data, const acul::string &title, i32 width, i32 height,
                               WindowFlags flags);

            void destroy(WindowData *);
            void update_buffer_scale_from_outputs(WaylandWindowData *window);

            void show_window(WindowData *window_data);
            void hide_window(WindowData *window_data);

            acul::string get_window_title(WindowData *window_data);
            void set_window_title(WindowData *window_data, const acul::string &title);

            void enable_fullscreen(WindowData *window_data);
            void disable_fullscreen(WindowData *window_data);

            acul::point2D<i32> get_cursor_position(WindowData *window_data);
            void set_cursor_position(WindowData *window_data, acul::point2D<i32> position);

            void hide_cursor(WindowData *window_data);
            void show_cursor(Window *, WindowData *window_data);

            void poll_events();
            void wait_events();
            void wait_events_timeout();
            void push_empty_event();

            acul::point2D<i32> get_window_position(WindowData *window);
            void set_window_position(WindowData *window, acul::point2D<i32> position);

            f32 get_dpi(WindowData *);
            acul::point2D<i32> get_window_size(const Window &window);

            acul::string get_clipboard_string();
            void set_clipboard_string(const acul::string &text);

            void set_window_icon(WindowData *, const acul::vector<Image> &);
            MonitorInfo get_primary_monitor_info();

            void center_window(WindowData *window);
            void update_resize_limit(WindowData *window);

            void minimize_window(WindowData *window);
            void maximize_window(WindowData *window);
        } // namespace wayland
    } // namespace platform
} // namespace awin