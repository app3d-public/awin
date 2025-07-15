#pragma once

#include <acul/api.hpp>
#include <acul/list.hpp>
#include <acul/map.hpp>
#include <linux/input-event-codes.h>
#include "../linux_pd.hpp"
#include "loaders.hpp"
// Client
#define wl_display_connect           awin::platform::wayland::ctx.wl.client.wl_display_connect
#define wl_display_flush             awin::platform::wayland::ctx.wl.client.wl_display_flush
#define wl_display_cancel_read       awin::platform::wayland::ctx.wl.client.wl_display_cancel_read
#define wl_display_dispatch_pending  awin::platform::wayland::ctx.wl.client.wl_display_dispatch_pending
#define wl_display_read_events       awin::platform::wayland::ctx.wl.client.wl_display_read_events
#define wl_display_disconnect        awin::platform::wayland::ctx.wl.client.wl_display_disconnect
#define wl_display_roundtrip         awin::platform::wayland::ctx.wl.client.wl_display_roundtrip
#define wl_display_get_fd            awin::platform::wayland::ctx.wl.client.wl_display_get_fd
#define wl_display_prepare_read      awin::platform::wayland::ctx.wl.client.wl_display_prepare_read
#define wl_proxy_marshal             awin::platform::wayland::ctx.wl.client.wl_proxy_marshal
#define wl_proxy_add_listener        awin::platform::wayland::ctx.wl.client.wl_proxy_add_listener
#define wl_proxy_destroy             awin::platform::wayland::ctx.wl.client.wl_proxy_destroy
#define wl_proxy_marshal_constructor awin::platform::wayland::ctx.wl.client.wl_proxy_marshal_constructor
#define wl_proxy_marshal_constructor_versioned \
    awin::platform::wayland::ctx.wl.client.wl_proxy_marshal_constructor_versioned
#define wl_proxy_get_user_data awin::platform::wayland::ctx.wl.client.wl_proxy_get_user_data
#define wl_proxy_set_user_data awin::platform::wayland::ctx.wl.client.wl_proxy_set_user_data
#define wl_proxy_get_tag       awin::platform::wayland::ctx.wl.client.wl_proxy_get_tag
#define wl_proxy_set_tag       awin::platform::wayland::ctx.wl.client.wl_proxy_set_tag
#define wl_proxy_get_version   awin::platform::wayland::ctx.wl.client.wl_proxy_get_version
#define wl_proxy_marshal_flags awin::platform::wayland::ctx.wl.client.wl_proxy_marshal_flags
// wayland-cursor
#define wl_cursor_theme_load       awin::platform::wayland::ctx.wl.cursor.wl_cursor_theme_load
#define wl_cursor_theme_destroy    awin::platform::wayland::ctx.wl.cursor.wl_cursor_theme_destroy
#define wl_cursor_theme_get_cursor awin::platform::wayland::ctx.wl.cursor.wl_cursor_theme_get_cursor
#define wl_cursor_image_get_buffer awin::platform::wayland::ctx.wl.cursor.wl_cursor_image_get_buffer
// XKB
#define xkb_compose_table_new_from_locale awin::platform::wayland::ctx.wl.xkb.xkb_compose_table_new_from_locale
#define xkb_compose_table_unref           awin::platform::wayland::ctx.wl.xkb.xkb_compose_table_unref
#define xkb_compose_state_new             awin::platform::wayland::ctx.wl.xkb.xkb_compose_state_new
#define xkb_compose_state_unref           awin::platform::wayland::ctx.wl.xkb.xkb_compose_state_unref
#define xkb_compose_state_feed            awin::platform::wayland::ctx.wl.xkb.xkb_compose_state_feed
#define xkb_compose_state_get_status      awin::platform::wayland::ctx.wl.xkb.xkb_compose_state_get_status
#define xkb_compose_state_get_one_sym     awin::platform::wayland::ctx.wl.xkb.xkb_compose_state_get_one_sym
#define xkb_context_new                   awin::platform::wayland::ctx.wl.xkb.xkb_context_new
#define xkb_context_unref                 awin::platform::wayland::ctx.wl.xkb.xkb_context_unref
#define xkb_keymap_new_from_string        awin::platform::wayland::ctx.wl.xkb.xkb_keymap_new_from_string
#define xkb_keymap_unref                  awin::platform::wayland::ctx.wl.xkb.xkb_keymap_unref
#define xkb_keymap_mod_get_index          awin::platform::wayland::ctx.wl.xkb.xkb_keymap_mod_get_index
#define xkb_keymap_key_repeats            awin::platform::wayland::ctx.wl.xkb.xkb_keymap_key_repeats
#define xkb_keymap_key_get_syms_by_level  awin::platform::wayland::ctx.wl.xkb.xkb_keymap_key_get_syms_by_level
#define xkb_state_new                     awin::platform::wayland::ctx.wl.xkb.xkb_state_new
#define xkb_state_unref                   awin::platform::wayland::ctx.wl.xkb.xkb_state_unref
#define xkb_state_key_get_syms            awin::platform::wayland::ctx.wl.xkb.xkb_state_key_get_syms
#define xkb_state_update_mask             awin::platform::wayland::ctx.wl.xkb.xkb_state_update_mask
#define xkb_state_key_get_layout          awin::platform::wayland::ctx.wl.xkb.xkb_state_key_get_layout
#define xkb_state_mod_index_is_active     awin::platform::wayland::ctx.wl.xkb.xkb_state_mod_index_is_active
#define xkb_keysym_to_utf32               awin::platform::wayland::ctx.wl.xkb.xkb_keysym_to_utf32
#define xkb_keysym_to_utf8                awin::platform::wayland::ctx.wl.xkb.xkb_keysym_to_utf8

struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_subcompositor;
struct wl_shm;
struct wl_seat;
struct wl_pointer;
struct wl_keyboard;
struct wl_data_device_manager;
struct wl_data_device;
struct xdg_wm_base;
struct wp_viewporter;
struct wp_fractional_scale_manager_v1;
struct wl_data_offer;
struct wl_output;
struct wl_cursor_theme;
struct wl_surface;
struct wp_viewport;
struct wl_subsurface;
struct wl_data_source;
struct zwp_idle_inhibit_manager_v1;
struct zwp_relative_pointer_manager_v1;
struct zwp_relative_pointer_v1;
struct zwp_pointer_constraints_v1;
struct xdg_activation_v1;
struct xdg_surface;
struct xdg_toplevel;
struct zxdg_toplevel_decoration_v1;
struct zxdg_decoration_manager_v1;
struct zwp_idle_inhibitor_v1;
struct wp_fractional_scale_v1;
struct wl_cursor_image
{
    u32 width;
    u32 height;
    u32 hotspot_x;
    u32 hotspot_y;
    u32 delay;
};
struct wl_cursor
{
    unsigned int image_count;
    struct wl_cursor_image **images;
    char *name;
};

#define libdecor_new                        awin::platform::wayland::ctx.wl.libdecor.libdecor_new
#define libdecor_unref                      awin::platform::wayland::ctx.wl.libdecor.libdecor_unref
#define libdecor_get_fd                     awin::platform::wayland::ctx.wl.libdecor.libdecor_get_fd
#define libdecor_dispatch                   awin::platform::wayland::ctx.wl.libdecor.libdecor_dispatch
#define libdecor_decorate                   awin::platform::wayland::ctx.wl.libdecor.libdecor_decorate
#define libdecor_frame_unref                awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_unref
#define libdecor_frame_set_app_id           awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_app_id
#define libdecor_frame_set_title            awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_title
#define libdecor_frame_set_minimized        awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_minimized
#define libdecor_frame_set_fullscreen       awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_fullscreen
#define libdecor_frame_unset_fullscreen     awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_unset_fullscreen
#define libdecor_frame_map                  awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_map
#define libdecor_frame_commit               awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_commit
#define libdecor_frame_set_min_content_size awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_min_content_size
#define libdecor_frame_set_max_content_size awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_max_content_size
#define libdecor_frame_set_maximized        awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_maximized
#define libdecor_frame_unset_maximized      awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_unset_maximized
#define libdecor_frame_set_capabilities     awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_capabilities
#define libdecor_frame_unset_capabilities   awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_unset_capabilities
#define libdecor_frame_set_visibility       awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_set_visibility
#define libdecor_frame_get_xdg_toplevel     awin::platform::wayland::ctx.wl.libdecor.libdecor_frame_get_xdg_toplevel
#define libdecor_configuration_get_content_size \
    awin::platform::wayland::ctx.wl.libdecor.libdecor_configuration_get_content_size
#define libdecor_configuration_get_window_state \
    awin::platform::wayland::ctx.wl.libdecor.libdecor_configuration_get_window_state
#define libdecor_state_new  awin::platform::wayland::ctx.wl.libdecor.libdecor_state_new
#define libdecor_state_free awin::platform::wayland::ctx.wl.libdecor.libdecor_state_free

namespace awin
{
    namespace platform
    {
        namespace wayland
        {
            struct Offer
            {
                wl_data_offer *offer;
                bool text_plain_utf8;
                bool text_uri_list;
            };

            struct WaylandLoader
            {
                ClientLoader client;
                XKBLoader xkb;
                WaylandCursorLoader cursor;
                LibdecorLoader libdecor;
            };

            struct Output
            {
                acul::string name;
                u32 name_id;
                wl_output *handle;
                acul::point2D<i32> pos, physical_size, dimensions;
                int current_mode;
                i32 scale;
                acul::vector<struct WaylandWindowData *> windows;
            };

            struct OutputScale
            {
                wl_output *output;
                i32 factor;
            };

            extern APPLIB_API struct Context
            {
                WaylandLoader wl;

                wl_display *display;
                wl_registry *registry;
                wl_compositor *compositor;
                wl_subcompositor *subcompositor;
                wl_shm *shm;
                wl_seat *seat;
                wl_pointer *pointer;
                wl_keyboard *keyboard;
                wl_data_device_manager *data_device_manager;
                wl_data_device *data_device;
                xdg_wm_base *wm_base;
                zxdg_decoration_manager_v1 *decoration_manager;
                wp_viewporter *viewporter;
                zwp_relative_pointer_manager_v1 *relative_pointer_manager;
                zwp_idle_inhibit_manager_v1 *idle_inhibit_manager;
                wp_fractional_scale_manager_v1 *fractional_scale_manager;

                int key_repeat_timer_fd, key_repeat_scancode;
                i32 key_repeat_rate, key_repeat_delay;
                u32 serial;
                u32 pointer_enter_serial;
                struct WaylandWindowData *pointer_focus, *keyboard_focus;
                const char *cursor_previous_name;
                const char *tag;
                wl_cursor_theme *cursor_theme;
                wl_cursor_theme *cursor_theme_hi_dpi;
                wl_surface *cursor_surface;
                int cursor_timer_fd;
                acul::vector<Output> outputs;
                acul::vector<Offer> offers;
                wl_data_offer *selection_offer;
                wl_data_source *selection_source;

                struct
                {
                    xkb_context *context;
                    xkb_keymap *keymap;
                    xkb_state *state;

                    xkb_compose_state *compose_state;

                    xkb_mod_index_t control_index;
                    xkb_mod_index_t alt_index;
                    xkb_mod_index_t shift_index;
                    xkb_mod_index_t super_index;
                    xkb_mod_index_t caps_lock_index;
                    xkb_mod_index_t num_lock_index;
                    io::KeyMode modifiers;
                } xkb;

                struct
                {
                    ::libdecor *context;
                    wl_callback *callback;
                    bool ready;
                } libdecor;

                acul::map<i16, io::Key> keymap{{KEY_SPACE, io::Key::Space},
                                               {KEY_APOSTROPHE, io::Key::Apostroph},
                                               {KEY_COMMA, io::Key::Comma},
                                               {KEY_MINUS, io::Key::Minus},
                                               {KEY_DOT, io::Key::Period},
                                               {KEY_SLASH, io::Key::Slash},
                                               {KEY_0, io::Key::D0},
                                               {KEY_1, io::Key::D1},
                                               {KEY_2, io::Key::D2},
                                               {KEY_3, io::Key::D3},
                                               {KEY_4, io::Key::D4},
                                               {KEY_5, io::Key::D5},
                                               {KEY_6, io::Key::D6},
                                               {KEY_7, io::Key::D7},
                                               {KEY_8, io::Key::D8},
                                               {KEY_9, io::Key::D9},
                                               {KEY_SEMICOLON, io::Key::Semicolon},
                                               {KEY_EQUAL, io::Key::Equal},
                                               {KEY_A, io::Key::A},
                                               {KEY_B, io::Key::B},
                                               {KEY_C, io::Key::C},
                                               {KEY_D, io::Key::D},
                                               {KEY_E, io::Key::E},
                                               {KEY_F, io::Key::F},
                                               {KEY_G, io::Key::G},
                                               {KEY_H, io::Key::H},
                                               {KEY_I, io::Key::I},
                                               {KEY_J, io::Key::J},
                                               {KEY_K, io::Key::K},
                                               {KEY_L, io::Key::L},
                                               {KEY_M, io::Key::M},
                                               {KEY_N, io::Key::N},
                                               {KEY_O, io::Key::O},
                                               {KEY_P, io::Key::P},
                                               {KEY_Q, io::Key::Q},
                                               {KEY_R, io::Key::R},
                                               {KEY_S, io::Key::S},
                                               {KEY_T, io::Key::T},
                                               {KEY_U, io::Key::U},
                                               {KEY_V, io::Key::V},
                                               {KEY_W, io::Key::W},
                                               {KEY_X, io::Key::X},
                                               {KEY_Y, io::Key::Y},
                                               {KEY_Z, io::Key::Z},
                                               {KEY_LEFTBRACE, io::Key::LeftBrace},
                                               {KEY_BACKSLASH, io::Key::Backslash},
                                               {KEY_RIGHTBRACE, io::Key::RightBrace},
                                               {KEY_GRAVE, io::Key::GraveAccent},
                                               {KEY_ESC, io::Key::Escape},
                                               {KEY_ENTER, io::Key::Enter},
                                               {KEY_TAB, io::Key::Tab},
                                               {KEY_BACKSPACE, io::Key::Backspace},
                                               {KEY_INSERT, io::Key::Insert},
                                               {KEY_DELETE, io::Key::Delete},
                                               {KEY_RIGHT, io::Key::Right},
                                               {KEY_LEFT, io::Key::Left},
                                               {KEY_DOWN, io::Key::Down},
                                               {KEY_UP, io::Key::Up},
                                               {KEY_PAGEUP, io::Key::PageUp},
                                               {KEY_PAGEDOWN, io::Key::PageDown},
                                               {KEY_HOME, io::Key::Home},
                                               {KEY_END, io::Key::End},
                                               {KEY_CAPSLOCK, io::Key::CapsLock},
                                               {KEY_SCROLLLOCK, io::Key::ScrollLock},
                                               {KEY_NUMLOCK, io::Key::NumLock},
                                               {KEY_PRINT, io::Key::PrintScreen},
                                               {KEY_PAUSE, io::Key::Pause},
                                               {KEY_F1, io::Key::F1},
                                               {KEY_F2, io::Key::F2},
                                               {KEY_F3, io::Key::F3},
                                               {KEY_F4, io::Key::F4},
                                               {KEY_F5, io::Key::F5},
                                               {KEY_F6, io::Key::F6},
                                               {KEY_F7, io::Key::F7},
                                               {KEY_F8, io::Key::F8},
                                               {KEY_F9, io::Key::F9},
                                               {KEY_F10, io::Key::F10},
                                               {KEY_F11, io::Key::F11},
                                               {KEY_F12, io::Key::F12},
                                               {KEY_F13, io::Key::F13},
                                               {KEY_F14, io::Key::F14},
                                               {KEY_F15, io::Key::F15},
                                               {KEY_F16, io::Key::F16},
                                               {KEY_F17, io::Key::F17},
                                               {KEY_F18, io::Key::F18},
                                               {KEY_F19, io::Key::F19},
                                               {KEY_F20, io::Key::F20},
                                               {KEY_F21, io::Key::F21},
                                               {KEY_F22, io::Key::F22},
                                               {KEY_F23, io::Key::F23},
                                               {KEY_F24, io::Key::F24},
                                               {KEY_KP0, io::Key::KP0},
                                               {KEY_KP1, io::Key::KP1},
                                               {KEY_KP2, io::Key::KP2},
                                               {KEY_KP3, io::Key::KP3},
                                               {KEY_KP4, io::Key::KP4},
                                               {KEY_KP5, io::Key::KP5},
                                               {KEY_KP6, io::Key::KP6},
                                               {KEY_KP7, io::Key::KP7},
                                               {KEY_KP8, io::Key::KP8},
                                               {KEY_KP9, io::Key::KP9},
                                               {KEY_KPDOT, io::Key::KPDecimal},
                                               {KEY_KPSLASH, io::Key::KPDivide},
                                               {KEY_KPASTERISK, io::Key::KPMultiply},
                                               {KEY_KPMINUS, io::Key::KPSubtract},
                                               {KEY_KPPLUS, io::Key::KPAdd},
                                               {KEY_KPENTER, io::Key::KPEnter},
                                               {KEY_KPEQUAL, io::Key::KPEqual},
                                               {KEY_LEFTSHIFT, io::Key::LeftShift},
                                               {KEY_LEFTCTRL, io::Key::LeftControl},
                                               {KEY_LEFTALT, io::Key::LeftAlt},
                                               {KEY_LEFTMETA, io::Key::LeftSuper},
                                               {KEY_RIGHTSHIFT, io::Key::RightShift},
                                               {KEY_RIGHTCTRL, io::Key::RightControl},
                                               {KEY_RIGHTALT, io::Key::RightAlt},
                                               {KEY_RIGHTMETA, io::Key::RightSuper},
                                               {KEY_MENU, io::Key::Menu}};
            } ctx;

            struct WaylandCursor : Cursor::Platform
            {
                wl_cursor *handle;
                wl_buffer *buffer;
                acul::point2D<int> size, hot;
                int current_image;
            };

            void add_seat_listener(wl_seat *seat);
            void add_data_device_listener(wl_data_device *device);

            void init_pcall_data(LinuxPlatformCaller &caller);
            void init_wcall_data(LinuxWindowCaller &caller);
            void init_ccall_data(LinuxCursorCaller &caller);

            Cursor::Platform *create_cursor(Cursor::Type);
            void assign_cursor(Window *, Cursor::Platform *);
            void destroy_cursor(Cursor::Platform *);
            bool is_cursor_valid(const Cursor::Platform *);
        } // namespace wayland
    } // namespace platform

} // namespace awin