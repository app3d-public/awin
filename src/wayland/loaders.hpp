#pragma once

#include <acul/scalars.hpp>
#include <dlfcn.h>
#include <wayland-client-core.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon.h>

// Client
typedef int (*PFN_wl_display_flush)(struct wl_display *display);
typedef void (*PFN_wl_display_cancel_read)(struct wl_display *display);
typedef int (*PFN_wl_display_dispatch_pending)(struct wl_display *display);
typedef int (*PFN_wl_display_read_events)(struct wl_display *display);
typedef struct wl_display *(*PFN_wl_display_connect)(const char *);
typedef void (*PFN_wl_display_disconnect)(struct wl_display *);
typedef int (*PFN_wl_display_roundtrip)(struct wl_display *);
typedef int (*PFN_wl_display_get_fd)(struct wl_display *);
typedef int (*PFN_wl_display_prepare_read)(struct wl_display *);
typedef void (*PFN_wl_proxy_marshal)(struct wl_proxy *, u32, ...);
typedef int (*PFN_wl_proxy_add_listener)(struct wl_proxy *, void (**)(void), void *);
typedef void (*PFN_wl_proxy_destroy)(struct wl_proxy *);
typedef struct wl_proxy *(*PFN_wl_proxy_marshal_constructor)(struct wl_proxy *, u32, const struct wl_interface *, ...);
typedef struct wl_proxy *(*PFN_wl_proxy_marshal_constructor_versioned)(struct wl_proxy *, u32,
                                                                       const struct wl_interface *, u32, ...);
typedef void *(*PFN_wl_proxy_get_user_data)(struct wl_proxy *);
typedef void (*PFN_wl_proxy_set_user_data)(struct wl_proxy *, void *);
typedef void (*PFN_wl_proxy_set_tag)(struct wl_proxy *, const char *const *);
typedef const char *const *(*PFN_wl_proxy_get_tag)(struct wl_proxy *);
typedef u32 (*PFN_wl_proxy_get_version)(struct wl_proxy *);
typedef struct wl_proxy *(*PFN_wl_proxy_marshal_flags)(struct wl_proxy *, u32, const struct wl_interface *, u32, u32,
                                                       ...);

// XKB
typedef struct xkb_context *(*PFN_xkb_context_new)(enum xkb_context_flags);
typedef void (*PFN_xkb_context_unref)(struct xkb_context *);
typedef struct xkb_keymap *(*PFN_xkb_keymap_new_from_string)(struct xkb_context *, const char *, enum xkb_keymap_format,
                                                             enum xkb_keymap_compile_flags);
typedef void (*PFN_xkb_keymap_unref)(struct xkb_keymap *);
typedef xkb_mod_index_t (*PFN_xkb_keymap_mod_get_index)(struct xkb_keymap *, const char *);
typedef int (*PFN_xkb_keymap_key_repeats)(struct xkb_keymap *, xkb_keycode_t);
typedef int (*PFN_xkb_keymap_key_get_syms_by_level)(struct xkb_keymap *, xkb_keycode_t, xkb_layout_index_t,
                                                    xkb_level_index_t, const xkb_keysym_t **);
typedef struct xkb_state *(*PFN_xkb_state_new)(struct xkb_keymap *);
typedef void (*PFN_xkb_state_unref)(struct xkb_state *);
typedef int (*PFN_xkb_state_key_get_syms)(struct xkb_state *, xkb_keycode_t, const xkb_keysym_t **);
typedef enum xkb_state_component (*PFN_xkb_state_update_mask)(struct xkb_state *, xkb_mod_mask_t, xkb_mod_mask_t,
                                                              xkb_mod_mask_t, xkb_layout_index_t, xkb_layout_index_t,
                                                              xkb_layout_index_t);
typedef xkb_layout_index_t (*PFN_xkb_state_key_get_layout)(struct xkb_state *, xkb_keycode_t);
typedef int (*PFN_xkb_state_mod_index_is_active)(struct xkb_state *, xkb_mod_index_t, enum xkb_state_component);
typedef uint32_t (*PFN_xkb_keysym_to_utf32)(xkb_keysym_t);
typedef int (*PFN_xkb_keysym_to_utf8)(xkb_keysym_t, char *, size_t);
typedef struct xkb_compose_table *(*PFN_xkb_compose_table_new_from_locale)(struct xkb_context *, const char *,
                                                                           enum xkb_compose_compile_flags);
typedef void (*PFN_xkb_compose_table_unref)(struct xkb_compose_table *);
typedef struct xkb_compose_state *(*PFN_xkb_compose_state_new)(struct xkb_compose_table *,
                                                               enum xkb_compose_state_flags);
typedef void (*PFN_xkb_compose_state_unref)(struct xkb_compose_state *);
typedef enum xkb_compose_feed_result (*PFN_xkb_compose_state_feed)(struct xkb_compose_state *, xkb_keysym_t);
typedef enum xkb_compose_status (*PFN_xkb_compose_state_get_status)(struct xkb_compose_state *);
typedef xkb_keysym_t (*PFN_xkb_compose_state_get_one_sym)(struct xkb_compose_state *);

// Cursor
typedef struct wl_cursor_theme *(*PFN_wl_cursor_theme_load)(const char *, int, struct wl_shm *);
typedef void (*PFN_wl_cursor_theme_destroy)(struct wl_cursor_theme *);
typedef struct wl_cursor *(*PFN_wl_cursor_theme_get_cursor)(struct wl_cursor_theme *, const char *);
typedef struct wl_buffer *(*PFN_wl_cursor_image_get_buffer)(struct wl_cursor_image *);

// libdecor
struct libdecor;
struct libdecor_frame;
struct libdecor_state;
struct libdecor_configuration;

enum libdecor_error
{
    LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
    LIBDECOR_ERROR_INVALID_FRAME_CONFIGURATION,
};

enum libdecor_window_state
{
    LIBDECOR_WINDOW_STATE_NONE = 0,
    LIBDECOR_WINDOW_STATE_ACTIVE = 1,
    LIBDECOR_WINDOW_STATE_MAXIMIZED = 2,
    LIBDECOR_WINDOW_STATE_FULLSCREEN = 4,
    LIBDECOR_WINDOW_STATE_TILED_LEFT = 8,
    LIBDECOR_WINDOW_STATE_TILED_RIGHT = 16,
    LIBDECOR_WINDOW_STATE_TILED_TOP = 32,
    LIBDECOR_WINDOW_STATE_TILED_BOTTOM = 64
};

enum libdecor_capabilities
{
    LIBDECOR_ACTION_MOVE = 1,
    LIBDECOR_ACTION_RESIZE = 2,
    LIBDECOR_ACTION_MINIMIZE = 4,
    LIBDECOR_ACTION_FULLSCREEN = 8,
    LIBDECOR_ACTION_CLOSE = 16
};

struct libdecor_interface
{
    void (*error)(struct libdecor *, enum libdecor_error, const char *);
    void (*reserved0)(void);
    void (*reserved1)(void);
    void (*reserved2)(void);
    void (*reserved3)(void);
    void (*reserved4)(void);
    void (*reserved5)(void);
    void (*reserved6)(void);
    void (*reserved7)(void);
    void (*reserved8)(void);
    void (*reserved9)(void);
};

struct libdecor_frame_interface
{
    void (*configure)(struct libdecor_frame *, struct libdecor_configuration *, void *);
    void (*close)(struct libdecor_frame *, void *);
    void (*commit)(struct libdecor_frame *, void *);
    void (*dismiss_popup)(struct libdecor_frame *, const char *, void *);
    void (*reserved0)(void);
    void (*reserved1)(void);
    void (*reserved2)(void);
    void (*reserved3)(void);
    void (*reserved4)(void);
    void (*reserved5)(void);
    void (*reserved6)(void);
    void (*reserved7)(void);
    void (*reserved8)(void);
    void (*reserved9)(void);
};

struct wl_callback;

typedef struct libdecor *(*PFN_libdecor_new)(struct wl_display *, const struct libdecor_interface *);
typedef void (*PFN_libdecor_unref)(struct libdecor *);
typedef int (*PFN_libdecor_get_fd)(struct libdecor *);
typedef int (*PFN_libdecor_dispatch)(struct libdecor *, int);
typedef struct libdecor_frame *(*PFN_libdecor_decorate)(struct libdecor *, struct wl_surface *,
                                                        const struct libdecor_frame_interface *, void *);
typedef void (*PFN_libdecor_frame_unref)(struct libdecor_frame *);
typedef void (*PFN_libdecor_frame_set_app_id)(struct libdecor_frame *, const char *);
typedef void (*PFN_libdecor_frame_set_title)(struct libdecor_frame *, const char *);
typedef void (*PFN_libdecor_frame_set_minimized)(struct libdecor_frame *);
typedef void (*PFN_libdecor_frame_set_fullscreen)(struct libdecor_frame *, struct wl_output *);
typedef void (*PFN_libdecor_frame_unset_fullscreen)(struct libdecor_frame *);
typedef void (*PFN_libdecor_frame_map)(struct libdecor_frame *);
typedef void (*PFN_libdecor_frame_commit)(struct libdecor_frame *, struct libdecor_state *,
                                          struct libdecor_configuration *);
typedef void (*PFN_libdecor_frame_set_min_content_size)(struct libdecor_frame *, int, int);
typedef void (*PFN_libdecor_frame_set_max_content_size)(struct libdecor_frame *, int, int);
typedef void (*PFN_libdecor_frame_set_maximized)(struct libdecor_frame *);
typedef void (*PFN_libdecor_frame_unset_maximized)(struct libdecor_frame *);
typedef void (*PFN_libdecor_frame_set_capabilities)(struct libdecor_frame *, enum libdecor_capabilities);
typedef void (*PFN_libdecor_frame_unset_capabilities)(struct libdecor_frame *, enum libdecor_capabilities);
typedef void (*PFN_libdecor_frame_set_visibility)(struct libdecor_frame *, bool visible);
typedef struct xdg_toplevel *(*PFN_libdecor_frame_get_xdg_toplevel)(struct libdecor_frame *);
typedef bool (*PFN_libdecor_configuration_get_content_size)(struct libdecor_configuration *, struct libdecor_frame *,
                                                            int *, int *);
typedef bool (*PFN_libdecor_configuration_get_window_state)(struct libdecor_configuration *,
                                                            enum libdecor_window_state *);
typedef struct libdecor_state *(*PFN_libdecor_state_new)(int, int);
typedef void (*PFN_libdecor_state_free)(struct libdecor_state *);

namespace awin
{
    namespace platform
    {
        namespace wayland
        {
            struct ILoader
            {
                void *handle = nullptr;

                ILoader(bool (*load)(ILoader *), bool (*valid)(ILoader *)) : _load(load), _valid(valid) {}

                bool load() { return _load(this); }

                bool valid() { return _valid ? true : _valid(this); }

                void unload()
                {
                    if (handle)
                    {
                        dlclose(handle);
                        handle = nullptr;
                    }
                }

            protected:
                bool (*_load)(ILoader *) = nullptr;
                bool (*_valid)(ILoader *) = nullptr;
            };

            struct ClientLoader : ILoader
            {
                PFN_wl_display_connect wl_display_connect;
                PFN_wl_display_flush wl_display_flush;
                PFN_wl_display_cancel_read wl_display_cancel_read;
                PFN_wl_display_dispatch_pending wl_display_dispatch_pending;
                PFN_wl_display_read_events wl_display_read_events;
                PFN_wl_display_disconnect wl_display_disconnect;
                PFN_wl_display_roundtrip wl_display_roundtrip;
                PFN_wl_display_get_fd wl_display_get_fd;
                PFN_wl_display_prepare_read wl_display_prepare_read;
                PFN_wl_proxy_marshal wl_proxy_marshal;
                PFN_wl_proxy_add_listener wl_proxy_add_listener;
                PFN_wl_proxy_destroy wl_proxy_destroy;
                PFN_wl_proxy_marshal_constructor wl_proxy_marshal_constructor;
                PFN_wl_proxy_marshal_constructor_versioned wl_proxy_marshal_constructor_versioned;
                PFN_wl_proxy_get_user_data wl_proxy_get_user_data;
                PFN_wl_proxy_set_user_data wl_proxy_set_user_data;
                PFN_wl_proxy_get_tag wl_proxy_get_tag;
                PFN_wl_proxy_set_tag wl_proxy_set_tag;
                PFN_wl_proxy_get_version wl_proxy_get_version;
                PFN_wl_proxy_marshal_flags wl_proxy_marshal_flags;

                ClientLoader();
            };

            struct XKBLoader: ILoader
            {
                PFN_xkb_context_new xkb_context_new;
                PFN_xkb_context_unref xkb_context_unref;
                PFN_xkb_keymap_new_from_string xkb_keymap_new_from_string;
                PFN_xkb_keymap_unref xkb_keymap_unref;
                PFN_xkb_keymap_mod_get_index xkb_keymap_mod_get_index;
                PFN_xkb_keymap_key_repeats xkb_keymap_key_repeats;
                PFN_xkb_keymap_key_get_syms_by_level xkb_keymap_key_get_syms_by_level;
                PFN_xkb_state_new xkb_state_new;
                PFN_xkb_state_unref xkb_state_unref;
                PFN_xkb_state_key_get_syms xkb_state_key_get_syms;
                PFN_xkb_state_update_mask xkb_state_update_mask;
                PFN_xkb_state_key_get_layout xkb_state_key_get_layout;
                PFN_xkb_state_mod_index_is_active xkb_state_mod_index_is_active;
                PFN_xkb_keysym_to_utf32 xkb_keysym_to_utf32;
                PFN_xkb_keysym_to_utf8 xkb_keysym_to_utf8;
                PFN_xkb_compose_table_new_from_locale xkb_compose_table_new_from_locale;
                PFN_xkb_compose_table_unref xkb_compose_table_unref;
                PFN_xkb_compose_state_new xkb_compose_state_new;
                PFN_xkb_compose_state_unref xkb_compose_state_unref;
                PFN_xkb_compose_state_feed xkb_compose_state_feed;
                PFN_xkb_compose_state_get_status xkb_compose_state_get_status;
                PFN_xkb_compose_state_get_one_sym xkb_compose_state_get_one_sym;

                XKBLoader();
            };

            struct WaylandCursorLoader: ILoader
            {
                PFN_wl_cursor_theme_load wl_cursor_theme_load;
                PFN_wl_cursor_theme_destroy wl_cursor_theme_destroy;
                PFN_wl_cursor_theme_get_cursor wl_cursor_theme_get_cursor;
                PFN_wl_cursor_image_get_buffer wl_cursor_image_get_buffer;

                WaylandCursorLoader();
            };

            struct LibdecorLoader: ILoader
            {
                PFN_libdecor_new libdecor_new;
                PFN_libdecor_unref libdecor_unref;
                PFN_libdecor_get_fd libdecor_get_fd;
                PFN_libdecor_dispatch libdecor_dispatch;
                PFN_libdecor_decorate libdecor_decorate;
                PFN_libdecor_frame_unref libdecor_frame_unref;
                PFN_libdecor_frame_set_app_id libdecor_frame_set_app_id;
                PFN_libdecor_frame_set_title libdecor_frame_set_title;
                PFN_libdecor_frame_set_minimized libdecor_frame_set_minimized;
                PFN_libdecor_frame_set_fullscreen libdecor_frame_set_fullscreen;
                PFN_libdecor_frame_unset_fullscreen libdecor_frame_unset_fullscreen;
                PFN_libdecor_frame_map libdecor_frame_map;
                PFN_libdecor_frame_commit libdecor_frame_commit;
                PFN_libdecor_frame_set_min_content_size libdecor_frame_set_min_content_size;
                PFN_libdecor_frame_set_max_content_size libdecor_frame_set_max_content_size;
                PFN_libdecor_frame_set_maximized libdecor_frame_set_maximized;
                PFN_libdecor_frame_unset_maximized libdecor_frame_unset_maximized;
                PFN_libdecor_frame_set_capabilities libdecor_frame_set_capabilities;
                PFN_libdecor_frame_unset_capabilities libdecor_frame_unset_capabilities;
                PFN_libdecor_frame_set_visibility libdecor_frame_set_visibility;
                PFN_libdecor_frame_get_xdg_toplevel libdecor_frame_get_xdg_toplevel;
                PFN_libdecor_configuration_get_content_size libdecor_configuration_get_content_size;
                PFN_libdecor_configuration_get_window_state libdecor_configuration_get_window_state;
                PFN_libdecor_state_new libdecor_state_new;
                PFN_libdecor_state_free libdecor_state_free;

                LibdecorLoader();
            };
        } // namespace wayland
    } // namespace platform
} // namespace awin