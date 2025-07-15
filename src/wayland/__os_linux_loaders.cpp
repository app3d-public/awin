#include <acul/log.hpp>
#include "loaders.hpp"

#define LOAD_FUNCTION(name, loader) loader.name = (PFN_##name)dlsym(loader.handle, #name)

namespace awin
{
    namespace platform
    {
        namespace wayland
        {
            inline bool load_library(void *&handle, const char *name)
            {
                handle = dlopen(name, RTLD_LAZY);
                if (!handle)
                {
                    LOG_ERROR("Failed to load %s: %s", name, dlerror());
                    return false;
                }
                return true;
            }

            bool load_wayland_client(ILoader *loader)
            {
                if (!load_library(loader->handle, "libwayland-client.so.0")) return false;
                auto &handle = *(ClientLoader *)loader;
                LOAD_FUNCTION(wl_display_connect, handle);
                LOAD_FUNCTION(wl_display_flush, handle);
                LOAD_FUNCTION(wl_display_cancel_read, handle);
                LOAD_FUNCTION(wl_display_dispatch_pending, handle);
                LOAD_FUNCTION(wl_display_read_events, handle);
                LOAD_FUNCTION(wl_display_disconnect, handle);
                LOAD_FUNCTION(wl_display_roundtrip, handle);
                LOAD_FUNCTION(wl_display_get_fd, handle);
                LOAD_FUNCTION(wl_display_prepare_read, handle);
                LOAD_FUNCTION(wl_proxy_marshal, handle);
                LOAD_FUNCTION(wl_proxy_add_listener, handle);
                LOAD_FUNCTION(wl_proxy_destroy, handle);
                LOAD_FUNCTION(wl_proxy_marshal_constructor, handle);
                LOAD_FUNCTION(wl_proxy_marshal_constructor_versioned, handle);
                LOAD_FUNCTION(wl_proxy_get_user_data, handle);
                LOAD_FUNCTION(wl_proxy_set_user_data, handle);
                LOAD_FUNCTION(wl_proxy_get_tag, handle);
                LOAD_FUNCTION(wl_proxy_set_tag, handle);
                LOAD_FUNCTION(wl_proxy_get_version, handle);
                LOAD_FUNCTION(wl_proxy_marshal_flags, handle);
                return true;
            }

            bool valid_wayland_client(ILoader *loader)
            {
                auto &wc = *(ClientLoader *)loader;
                return wc.wl_display_flush && wc.wl_display_cancel_read && wc.wl_display_dispatch_pending &&
                       wc.wl_display_read_events && wc.wl_display_disconnect && wc.wl_display_roundtrip &&
                       wc.wl_display_get_fd && wc.wl_display_prepare_read && wc.wl_proxy_marshal &&
                       wc.wl_proxy_add_listener && wc.wl_proxy_destroy && wc.wl_proxy_marshal_constructor &&
                       wc.wl_proxy_marshal_constructor_versioned && wc.wl_proxy_get_user_data &&
                       wc.wl_proxy_set_user_data && wc.wl_proxy_get_tag && wc.wl_proxy_set_tag;
            }

            ClientLoader::ClientLoader() : ILoader(load_wayland_client, valid_wayland_client) {}

            bool load_xkb_common(ILoader *loader)
            {
                if (!load_library(loader->handle, "libxkbcommon.so.0")) return false;
                auto &handle = *(XKBLoader *)loader;
                LOAD_FUNCTION(xkb_context_new, handle);
                LOAD_FUNCTION(xkb_context_unref, handle);
                LOAD_FUNCTION(xkb_keymap_new_from_string, handle);
                LOAD_FUNCTION(xkb_keymap_unref, handle);
                LOAD_FUNCTION(xkb_keymap_mod_get_index, handle);
                LOAD_FUNCTION(xkb_keymap_key_repeats, handle);
                LOAD_FUNCTION(xkb_keymap_key_get_syms_by_level, handle);
                LOAD_FUNCTION(xkb_state_new, handle);
                LOAD_FUNCTION(xkb_state_unref, handle);
                LOAD_FUNCTION(xkb_state_key_get_syms, handle);
                LOAD_FUNCTION(xkb_state_update_mask, handle);
                LOAD_FUNCTION(xkb_state_key_get_layout, handle);
                LOAD_FUNCTION(xkb_state_mod_index_is_active, handle);
                LOAD_FUNCTION(xkb_keysym_to_utf32, handle);
                LOAD_FUNCTION(xkb_keysym_to_utf8, handle);
                LOAD_FUNCTION(xkb_compose_table_new_from_locale, handle);
                LOAD_FUNCTION(xkb_compose_table_unref, handle);
                LOAD_FUNCTION(xkb_compose_state_new, handle);
                LOAD_FUNCTION(xkb_compose_state_unref, handle);
                LOAD_FUNCTION(xkb_compose_state_feed, handle);
                LOAD_FUNCTION(xkb_compose_state_get_status, handle);
                LOAD_FUNCTION(xkb_compose_state_get_one_sym, handle);
                return true;
            }

            bool valid_xkb_common(ILoader *loader)
            {
                auto &xkb = *(XKBLoader *)loader;
                return xkb.xkb_context_new && xkb.xkb_context_unref && xkb.xkb_keymap_new_from_string &&
                       xkb.xkb_keymap_unref && xkb.xkb_keymap_mod_get_index && xkb.xkb_keymap_key_repeats &&
                       xkb.xkb_keymap_key_get_syms_by_level && xkb.xkb_state_new && xkb.xkb_state_unref &&
                       xkb.xkb_state_key_get_syms && xkb.xkb_state_update_mask && xkb.xkb_state_key_get_layout &&
                       xkb.xkb_state_mod_index_is_active && xkb.xkb_compose_table_new_from_locale &&
                       xkb.xkb_compose_table_unref && xkb.xkb_compose_state_new && xkb.xkb_compose_state_unref &&
                       xkb.xkb_compose_state_feed && xkb.xkb_compose_state_get_status &&
                       xkb.xkb_compose_state_get_one_sym;
            }

            XKBLoader::XKBLoader() : ILoader(load_xkb_common, valid_xkb_common) {}

            bool load_wayland_cursor(ILoader *loader)
            {
                if (!load_library(loader->handle, "libwayland-cursor.so.0")) return false;
                auto &handle = *(WaylandCursorLoader *)loader;
                LOAD_FUNCTION(wl_cursor_theme_load, handle);
                LOAD_FUNCTION(wl_cursor_theme_destroy, handle);
                LOAD_FUNCTION(wl_cursor_theme_get_cursor, handle);
                LOAD_FUNCTION(wl_cursor_image_get_buffer, handle);
                return true;
            }

            bool valid_wayland_cursor(ILoader *loader)
            {
                auto &wc = *(WaylandCursorLoader *)loader;
                return wc.wl_cursor_theme_load && wc.wl_cursor_theme_destroy && wc.wl_cursor_theme_get_cursor &&
                       wc.wl_cursor_image_get_buffer;
            }

            WaylandCursorLoader::WaylandCursorLoader() : ILoader(load_wayland_cursor, valid_wayland_cursor) {}

            bool load_libdecor(ILoader *loader)
            {
                if (!load_library(loader->handle, "libdecor-0.so.0")) return false;
                auto &handle = *(LibdecorLoader *)loader;
                LOAD_FUNCTION(libdecor_new, handle);
                LOAD_FUNCTION(libdecor_unref, handle);
                LOAD_FUNCTION(libdecor_get_fd, handle);
                LOAD_FUNCTION(libdecor_dispatch, handle);
                LOAD_FUNCTION(libdecor_decorate, handle);
                LOAD_FUNCTION(libdecor_frame_unref, handle);
                LOAD_FUNCTION(libdecor_frame_set_app_id, handle);
                LOAD_FUNCTION(libdecor_frame_set_title, handle);
                LOAD_FUNCTION(libdecor_frame_set_minimized, handle);
                LOAD_FUNCTION(libdecor_frame_set_fullscreen, handle);
                LOAD_FUNCTION(libdecor_frame_unset_fullscreen, handle);
                LOAD_FUNCTION(libdecor_frame_map, handle);
                LOAD_FUNCTION(libdecor_frame_commit, handle);
                LOAD_FUNCTION(libdecor_frame_set_min_content_size, handle);
                LOAD_FUNCTION(libdecor_frame_set_max_content_size, handle);
                LOAD_FUNCTION(libdecor_frame_set_maximized, handle);
                LOAD_FUNCTION(libdecor_frame_unset_maximized, handle);
                LOAD_FUNCTION(libdecor_frame_set_capabilities, handle);
                LOAD_FUNCTION(libdecor_frame_unset_capabilities, handle);
                LOAD_FUNCTION(libdecor_frame_set_visibility, handle);
                LOAD_FUNCTION(libdecor_frame_get_xdg_toplevel, handle);
                LOAD_FUNCTION(libdecor_configuration_get_content_size, handle);
                LOAD_FUNCTION(libdecor_configuration_get_window_state, handle);
                LOAD_FUNCTION(libdecor_state_new, handle);
                LOAD_FUNCTION(libdecor_state_free, handle);
                return true;
            }

            bool valid_libdecor(ILoader *loader)
            {
                auto &ld = *(LibdecorLoader *)loader;
                return ld.libdecor_new && ld.libdecor_unref && ld.libdecor_get_fd && ld.libdecor_dispatch &&
                       ld.libdecor_decorate && ld.libdecor_frame_unref && ld.libdecor_frame_set_app_id &&
                       ld.libdecor_frame_set_title && ld.libdecor_frame_set_minimized &&
                       ld.libdecor_frame_set_fullscreen && ld.libdecor_frame_unset_fullscreen &&
                       ld.libdecor_frame_map && ld.libdecor_frame_commit && ld.libdecor_frame_set_min_content_size &&
                       ld.libdecor_frame_set_max_content_size && ld.libdecor_frame_set_maximized &&
                       ld.libdecor_frame_unset_maximized && ld.libdecor_frame_set_capabilities &&
                       ld.libdecor_frame_unset_capabilities && ld.libdecor_frame_set_visibility &&
                       ld.libdecor_frame_get_xdg_toplevel && ld.libdecor_configuration_get_content_size &&
                       ld.libdecor_configuration_get_window_state && ld.libdecor_state_new && ld.libdecor_state_free;
            }

            LibdecorLoader::LibdecorLoader() : ILoader(load_libdecor, valid_libdecor) {}
        } // namespace wayland
    } // namespace platform
} // namespace awin