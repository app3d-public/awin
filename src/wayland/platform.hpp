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

                acul::map<i16, io::Key> keymap{{KEY_SPACE, io::Key::space},
                                               {KEY_APOSTROPHE, io::Key::apostroph},
                                               {KEY_COMMA, io::Key::comma},
                                               {KEY_MINUS, io::Key::minus},
                                               {KEY_DOT, io::Key::period},
                                               {KEY_SLASH, io::Key::slash},
                                               {KEY_0, io::Key::d0},
                                               {KEY_1, io::Key::d1},
                                               {KEY_2, io::Key::d2},
                                               {KEY_3, io::Key::d3},
                                               {KEY_4, io::Key::d4},
                                               {KEY_5, io::Key::d5},
                                               {KEY_6, io::Key::d6},
                                               {KEY_7, io::Key::d7},
                                               {KEY_8, io::Key::d8},
                                               {KEY_9, io::Key::d9},
                                               {KEY_SEMICOLON, io::Key::semicolon},
                                               {KEY_EQUAL, io::Key::equal},
                                               {KEY_A, io::Key::a},
                                               {KEY_B, io::Key::b},
                                               {KEY_C, io::Key::c},
                                               {KEY_D, io::Key::d},
                                               {KEY_E, io::Key::e},
                                               {KEY_F, io::Key::f},
                                               {KEY_G, io::Key::g},
                                               {KEY_H, io::Key::h},
                                               {KEY_I, io::Key::i},
                                               {KEY_J, io::Key::j},
                                               {KEY_K, io::Key::k},
                                               {KEY_L, io::Key::l},
                                               {KEY_M, io::Key::m},
                                               {KEY_N, io::Key::n},
                                               {KEY_O, io::Key::o},
                                               {KEY_P, io::Key::p},
                                               {KEY_Q, io::Key::q},
                                               {KEY_R, io::Key::r},
                                               {KEY_S, io::Key::s},
                                               {KEY_T, io::Key::t},
                                               {KEY_U, io::Key::u},
                                               {KEY_V, io::Key::v},
                                               {KEY_W, io::Key::w},
                                               {KEY_X, io::Key::x},
                                               {KEY_Y, io::Key::y},
                                               {KEY_Z, io::Key::z},
                                               {KEY_LEFTBRACE, io::Key::lbrace},
                                               {KEY_BACKSLASH, io::Key::backslash},
                                               {KEY_RIGHTBRACE, io::Key::rcontrol},
                                               {KEY_GRAVE, io::Key::grave_accent},
                                               {KEY_ESC, io::Key::escape},
                                               {KEY_ENTER, io::Key::enter},
                                               {KEY_TAB, io::Key::tab},
                                               {KEY_BACKSPACE, io::Key::backspace},
                                               {KEY_INSERT, io::Key::insert},
                                               {KEY_DELETE, io::Key::del},
                                               {KEY_RIGHT, io::Key::right},
                                               {KEY_LEFT, io::Key::left},
                                               {KEY_DOWN, io::Key::down},
                                               {KEY_UP, io::Key::up},
                                               {KEY_PAGEUP, io::Key::page_up},
                                               {KEY_PAGEDOWN, io::Key::page_down},
                                               {KEY_HOME, io::Key::home},
                                               {KEY_END, io::Key::end},
                                               {KEY_CAPSLOCK, io::Key::caps_lock},
                                               {KEY_SCROLLLOCK, io::Key::scroll_lock},
                                               {KEY_NUMLOCK, io::Key::num_lock},
                                               {KEY_PRINT, io::Key::print_screen},
                                               {KEY_PAUSE, io::Key::pause},
                                               {KEY_F1, io::Key::f1},
                                               {KEY_F2, io::Key::f2},
                                               {KEY_F3, io::Key::f3},
                                               {KEY_F4, io::Key::f4},
                                               {KEY_F5, io::Key::f5},
                                               {KEY_F6, io::Key::f6},
                                               {KEY_F7, io::Key::f7},
                                               {KEY_F8, io::Key::f8},
                                               {KEY_F9, io::Key::f9},
                                               {KEY_F10, io::Key::f10},
                                               {KEY_F11, io::Key::f11},
                                               {KEY_F12, io::Key::f12},
                                               {KEY_F13, io::Key::f13},
                                               {KEY_F14, io::Key::f14},
                                               {KEY_F15, io::Key::f15},
                                               {KEY_F16, io::Key::f16},
                                               {KEY_F17, io::Key::f17},
                                               {KEY_F18, io::Key::f18},
                                               {KEY_F19, io::Key::f19},
                                               {KEY_F20, io::Key::f20},
                                               {KEY_F21, io::Key::f21},
                                               {KEY_F22, io::Key::f22},
                                               {KEY_F23, io::Key::f23},
                                               {KEY_F24, io::Key::f24},
                                               {KEY_KP0, io::Key::kp_0},
                                               {KEY_KP1, io::Key::kp_1},
                                               {KEY_KP2, io::Key::kp_2},
                                               {KEY_KP3, io::Key::kp_3},
                                               {KEY_KP4, io::Key::kp_4},
                                               {KEY_KP5, io::Key::kp_5},
                                               {KEY_KP6, io::Key::kp_6},
                                               {KEY_KP7, io::Key::kp_7},
                                               {KEY_KP8, io::Key::kp_8},
                                               {KEY_KP9, io::Key::kp_9},
                                               {KEY_KPDOT, io::Key::kp_decimal},
                                               {KEY_KPSLASH, io::Key::kp_divide},
                                               {KEY_KPASTERISK, io::Key::kp_multiply},
                                               {KEY_KPMINUS, io::Key::kp_subtract},
                                               {KEY_KPPLUS, io::Key::kp_add},
                                               {KEY_KPENTER, io::Key::kp_enter},
                                               {KEY_KPEQUAL, io::Key::kp_equal},
                                               {KEY_LEFTSHIFT, io::Key::lshift},
                                               {KEY_LEFTCTRL, io::Key::lcontrol},
                                               {KEY_LEFTALT, io::Key::lalt},
                                               {KEY_LEFTMETA, io::Key::lsuper},
                                               {KEY_RIGHTSHIFT, io::Key::rshift},
                                               {KEY_RIGHTCTRL, io::Key::rcontrol},
                                               {KEY_RIGHTALT, io::Key::ralt},
                                               {KEY_RIGHTMETA, io::Key::rsuper},
                                               {KEY_MENU, io::Key::menu}};
#ifdef AWIN_TEST_BUILD
                bool is_surface_placeholder_enabled = false;
#endif
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