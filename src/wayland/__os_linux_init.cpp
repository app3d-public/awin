#include <acul/list.hpp>
#include <acul/log.hpp>
#include <sys/timerfd.h>
#include <xkbcommon/xkbcommon.h>
#include "../env.hpp"
#include "../linux_pd.hpp"
#include "generators/redifinition.h"
#include "loaders.hpp"
#include "platform.hpp"
#include "window.hpp"
//
#include "fractional-scale-v1-client-protocol.h"
#include "idle-inhibit-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"
#include "viewporter-client-protocol.h"
#include "wayland-client-protocol.h"
#include "xdg-activation-v1-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

namespace awin
{
    namespace platform
    {
        namespace wayland
        {
            Context ctx;

            static void wm_base_handle_ping(void *user_data, xdg_wm_base *wm_base, u32 serial)
            {
                xdg_wm_base_pong(wm_base, serial);
            }

            static const struct xdg_wm_base_listener wm_base_listener = {wm_base_handle_ping};

            static void output_handle_geometry(void *user_data, wl_output *, i32 x, i32 y, i32 physical_width,
                                               i32 physical_height, i32 subpixel, const char *make, const char *model,
                                               i32 transform)
            {
                Output *output = (Output *)user_data;
                output->pos.x = x;
                output->pos.y = y;
                output->physical_size.x = physical_width;
                output->physical_size.y = physical_height;
                if (output->name.empty()) output->name = acul::format("%s %s", make, model);
            }

            static void output_handle_mode(void *user_data, wl_output *, u32 flags, i32 width, i32 height, i32 refresh)
            {
                if (!(flags & WL_OUTPUT_MODE_CURRENT)) return;
                Output *output = (Output *)user_data;
                output->dimensions.x = width;
                output->dimensions.y = height;
            }

            static void output_handle_done(void *user_data, wl_output *)
            {
                Output *output = (Output *)user_data;

                if (output->physical_size.x <= 0 || output->physical_size.y <= 0)
                {
                    // If Wayland does not provide a physical size, assume the default 96 DPI
                    output->physical_size.x = (i32)(output->dimensions.x * 25.4f / 96.f);
                    output->physical_size.y = (i32)(output->dimensions.y * 25.4f / 96.f);
                }
            }

            void update_buffer_scale_from_outputs(WaylandWindowData *window)
            {
                if (wl_compositor_get_version(ctx.compositor) < WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION) return;
                if (!window->scale_framebuffer || window->fractional_scale) return;
                // Get the scale factor from the highest scale monitor.
                i32 max_scale = 1;
                for (const auto &scale : window->output_scales) max_scale = std::max(max_scale, scale.factor);

                // Only change the framebuffer size if the scale changed.
                if (window->buffer_scale != max_scale)
                {
                    window->buffer_scale = max_scale;
                    wl_surface_set_buffer_scale(window->surface, max_scale);
                    acul::events::dispatch_event_group<DpiChangedEvent>(event_registry.dpi_changed, window->owner,
                                                                        max_scale, max_scale);
                }
            }

            static void output_handle_scale(void *user_data, wl_output *, i32 factor)
            {
                Output *output = (Output *)user_data;
                output->scale = factor;

                for (auto *window : output->windows)
                {
                    for (auto &output_scale : window->output_scales)
                    {
                        if (output_scale.output == output->handle)
                        {
                            output_scale.factor = output->scale;
                            update_buffer_scale_from_outputs(window);
                            break;
                        }
                    }
                }
            }

            void output_handle_name(void *user_data, wl_output *, const char *name)
            {
                Output *output = (Output *)user_data;
                output->name = name;
            }

            void output_handle_description(void *, wl_output *, const char *) {}

            static const struct wl_output_listener output_listener = {
                output_handle_geometry, output_handle_mode, output_handle_done,
                output_handle_scale,    output_handle_name, output_handle_description,
            };

            static void add_output(u32 name, u32 version)
            {
                if (version < 2)
                {
                    LOG_ERROR("Unsupported Wayland output interface version");
                    return;
                }

                version = std::min(version, (u32)WL_OUTPUT_NAME_SINCE_VERSION);

                wl_output *handle = (wl_output *)wl_registry_bind(ctx.registry, name, &wl_output_interface, version);
                if (!handle) return;

                ctx.outputs.emplace_back();
                Output &output = ctx.outputs.back();
                output.name_id = name;
                output.handle = handle;
                output.scale = 1;

                wl_proxy_set_tag(reinterpret_cast<wl_proxy *>(handle), &ctx.tag);
                wl_output_add_listener(handle, &output_listener, &output);
            }

            static void registry_handle_global(void *user_data, wl_registry *registry, u32 name, const char *interface,
                                               u32 version)
            {
                if (strcmp(interface, "wl_compositor") == 0)
                {
                    ctx.compositor = (wl_compositor *)wl_registry_bind(registry, name, &wl_compositor_interface,
                                                                       std::min(3U, version));
                }
                else if (strcmp(interface, "wl_subcompositor") == 0)
                    ctx.subcompositor =
                        (wl_subcompositor *)wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
                else if (strcmp(interface, "wl_shm") == 0)
                    ctx.shm = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, 1);
                else if (strcmp(interface, "wl_output") == 0)
                    add_output(name, version);
                else if (strcmp(interface, "wl_seat") == 0)
                {
                    if (!ctx.seat)
                    {
                        ctx.seat =
                            (wl_seat *)wl_registry_bind(registry, name, &wl_seat_interface, std::min(4U, version));
                        add_seat_listener(ctx.seat);

                        if (wl_seat_get_version(ctx.seat) >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
                            ctx.key_repeat_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
                    }
                }
                else if (strcmp(interface, "wl_data_device_manager") == 0)
                {
                    if (!ctx.data_device_manager)
                        ctx.data_device_manager = (wl_data_device_manager *)wl_registry_bind(
                            registry, name, &wl_data_device_manager_interface, 1);
                }
                else if (strcmp(interface, "xdg_wm_base") == 0)
                {
                    ctx.wm_base = (xdg_wm_base *)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
                    xdg_wm_base_add_listener(ctx.wm_base, &wm_base_listener, NULL);
                }
                else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0)
                    ctx.decoration_manager = (zxdg_decoration_manager_v1 *)wl_registry_bind(
                        registry, name, &zxdg_decoration_manager_v1_interface, 1);
                else if (strcmp(interface, "wp_viewporter") == 0)
                    ctx.viewporter = (wp_viewporter *)wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
                else if (strcmp(interface, "zwp_relative_pointer_manager_v1") == 0)
                    ctx.relative_pointer_manager = (zwp_relative_pointer_manager_v1 *)wl_registry_bind(
                        registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
                else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0)
                    ctx.idle_inhibit_manager = (zwp_idle_inhibit_manager_v1 *)wl_registry_bind(
                        registry, name, &zwp_idle_inhibit_manager_v1_interface, 1);
                else if (strcmp(interface, "wp_fractional_scale_manager_v1") == 0)
                    ctx.fractional_scale_manager = (wp_fractional_scale_manager_v1 *)wl_registry_bind(
                        registry, name, &wp_fractional_scale_manager_v1_interface, 1);
            }

            static void registry_handle_global_remove(void *user_data, wl_registry *registry, u32 name)
            {
                auto it = std::find_if(ctx.outputs.begin(), ctx.outputs.end(),
                                       [name](const Output &output) { return output.name_id == name; });
                if (it == ctx.outputs.end()) return;
                wl_output_destroy(it->handle);
                ctx.outputs.erase(it);
            }

            static const struct wl_registry_listener registry_listener = {registry_handle_global,
                                                                          registry_handle_global_remove};

            void libdecor_handle_error(libdecor *context, libdecor_error error, const char *message)
            {
                LOG_ERROR("Wayland: libdecor error %u: %s", error, message);
            }

            static const struct libdecor_interface libdecor_interface = {libdecor_handle_error};

            static void libdecor_ready_callback(void *user_data, wl_callback *callback, u32 time)
            {
                ctx.libdecor.ready = true;
                assert(ctx.libdecor.callback == callback);
                wl_callback_destroy(ctx.libdecor.callback);
                ctx.libdecor.callback = NULL;
            }

            static const struct wl_callback_listener libdecor_ready_listener = {libdecor_ready_callback};

            static bool load_cursor_theme()
            {
                int cursor_size = 16;

                const char *size_string = getenv("XCURSOR_SIZE");
                if (size_string)
                {
                    errno = 0;
                    const long cursor_size_long = strtol(size_string, NULL, 10);
                    if (errno == 0 && cursor_size_long > 0 && cursor_size_long < INT_MAX)
                        cursor_size = (int)cursor_size_long;
                }

                const char *theme_name = getenv("XCURSOR_THEME");

                ctx.cursor_theme = wl_cursor_theme_load(theme_name, cursor_size, ctx.shm);
                if (!ctx.cursor_theme)
                {
                    LOG_ERROR("Wayland: Failed to load default cursor theme");
                    return false;
                }

                // If this happens to be NULL, we just fallback to the scale=1 version.
                ctx.cursor_theme_hi_dpi = wl_cursor_theme_load(theme_name, cursor_size * 2, ctx.shm);
                ctx.cursor_surface = wl_compositor_create_surface(ctx.compositor);
                ctx.cursor_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
                return true;
            }

            inline bool load_module(ILoader *loader, const char *name, bool required = true)
            {
                LOG_INFO("Loading module: %s", name);
                if (!loader->load()) return false;
                if (!loader->valid())
                {
                    LOG_ERROR("Failed to load %s entry point", name);
                    loader->unload();
                    return required ? false : true;
                }
                return true;
            }

            bool init_platform()
            {
                // These must be set before any failure checks
                ctx.key_repeat_timer_fd = -1;
                ctx.tag = "awin";

                if (!load_module(&ctx.wl.client, "wayland-client") ||    // wl
                    !load_module(&ctx.wl.xkb, "xkb") ||                  // xkb
                    !load_module(&ctx.wl.libdecor, "libdecor", false) || // libdecor
                    !load_module(&ctx.wl.cursor, "wayland-cursor")       // wl-cursor
                )
                    return false;

                ctx.display = wl_display_connect(NULL);
                if (!ctx.display)
                {
                    LOG_ERROR("Wayland: Failed to connect to wayland display");
                    return false;
                }

                ctx.registry = wl_display_get_registry(ctx.display);
                wl_registry_add_listener(ctx.registry, &registry_listener, NULL);

                ctx.xkb.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
                if (!ctx.xkb.context)
                {
                    LOG_ERROR("Wayland: Failed to initialize xkb context");
                    return false;
                }

                wl_display_roundtrip(ctx.display); // Sync so we got all registry objects
                wl_display_roundtrip(ctx.display); // Sync so we got all initial output events

                if (ctx.wl.libdecor.handle)
                {
                    ctx.libdecor.context = libdecor_new(ctx.display, &libdecor_interface);
                    if (ctx.libdecor.context)
                    {
                        // Perform an initial dispatch and flush to get the init started
                        libdecor_dispatch(ctx.libdecor.context, 0);

                        // Create sync point to "know" when libdecor is ready for use
                        ctx.libdecor.callback = wl_display_sync(ctx.display);
                        wl_callback_add_listener(ctx.libdecor.callback, &libdecor_ready_listener, NULL);
                    }
                }

                if (!ctx.wm_base)
                {
                    LOG_ERROR("Wayland: Failed to find xdg-shell in your compositor");
                    return false;
                }

                if (!ctx.shm)
                {
                    LOG_ERROR("Wayland: Failed to find wl_shm in your compositor");
                    return false;
                }

                if (!load_cursor_theme()) return false;

                if (ctx.seat && ctx.data_device_manager)
                {
                    ctx.data_device = wl_data_device_manager_get_data_device(ctx.data_device_manager, ctx.seat);
                    add_data_device_listener(ctx.data_device);
                }

                return true;
            }

            void destroy_platform()
            {
                if (ctx.libdecor.context)
                {
                    // Allow libdecor to finish receiving all its requested globals
                    // and ensure the associated sync callback object is destroyed
                    while (!ctx.libdecor.ready) wait_events();
                    libdecor_unref(ctx.libdecor.context);
                }

                if (ctx.wl.libdecor.handle) ctx.wl.libdecor.unload();
                if (ctx.xkb.compose_state) xkb_compose_state_unref(ctx.xkb.compose_state);
                if (ctx.xkb.keymap) xkb_keymap_unref(ctx.xkb.keymap);
                if (ctx.xkb.state) xkb_state_unref(ctx.xkb.state);
                if (ctx.xkb.context) xkb_context_unref(ctx.xkb.context);
                if (ctx.wl.xkb.handle) ctx.wl.xkb.unload();

                if (ctx.cursor_theme) wl_cursor_theme_destroy(ctx.cursor_theme);
                if (ctx.cursor_theme_hi_dpi) wl_cursor_theme_destroy(ctx.cursor_theme_hi_dpi);
                if (ctx.wl.cursor.handle) ctx.wl.cursor.unload();

                for (auto &offer : ctx.offers) wl_data_offer_destroy(offer.offer);

                if (ctx.cursor_surface) wl_surface_destroy(ctx.cursor_surface);
                if (ctx.subcompositor) wl_subcompositor_destroy(ctx.subcompositor);
                if (ctx.compositor) wl_compositor_destroy(ctx.compositor);
                if (ctx.shm) wl_shm_destroy(ctx.shm);
                if (ctx.viewporter) wp_viewporter_destroy(ctx.viewporter);
                if (ctx.decoration_manager) zxdg_decoration_manager_v1_destroy(ctx.decoration_manager);
                if (ctx.wm_base) xdg_wm_base_destroy(ctx.wm_base);
                if (ctx.selection_offer) wl_data_offer_destroy(ctx.selection_offer);
                if (ctx.selection_source) wl_data_source_destroy(ctx.selection_source);
                if (ctx.data_device) wl_data_device_destroy(ctx.data_device);
                if (ctx.data_device_manager) wl_data_device_manager_destroy(ctx.data_device_manager);
                if (ctx.pointer) wl_pointer_destroy(ctx.pointer);
                if (ctx.keyboard) wl_keyboard_destroy(ctx.keyboard);
                if (ctx.seat) wl_seat_destroy(ctx.seat);
                if (ctx.relative_pointer_manager) zwp_relative_pointer_manager_v1_destroy(ctx.relative_pointer_manager);
                if (ctx.idle_inhibit_manager) zwp_idle_inhibit_manager_v1_destroy(ctx.idle_inhibit_manager);
                if (ctx.fractional_scale_manager) wp_fractional_scale_manager_v1_destroy(ctx.fractional_scale_manager);
                if (ctx.registry) wl_registry_destroy(ctx.registry);
                if (ctx.display)
                {
                    wl_display_flush(ctx.display);
                    wl_display_disconnect(ctx.display);
                }

                if (ctx.key_repeat_timer_fd >= 0) close(ctx.key_repeat_timer_fd);
                if (ctx.cursor_timer_fd >= 0) close(ctx.cursor_timer_fd);
            }

            WindowData *alloc_window_data() { return acul::alloc<WaylandWindowData>(); }

            void init_pcall_data(LinuxPlatformCaller &caller)
            {
                caller.init_platform = init_platform;
                caller.destroy_platform = destroy_platform;
                caller.alloc_window_data = alloc_window_data;
                caller.poll_events = poll_events;
                caller.wait_events = wait_events;
                caller.wait_events_timeout = wait_events_timeout;
                caller.push_empty_event = push_empty_event;
                caller.get_dpi = get_dpi;
                caller.get_window_size = get_window_size;
                caller.get_clipboard_string = get_clipboard_string;
                caller.set_clipboard_string = set_clipboard_string;
                caller.get_primary_monitor_info = get_primary_monitor_info;
            }

            void init_wcall_data(LinuxWindowCaller &caller)
            {
                caller.destroy = destroy;
                caller.create_window = create_window;
                caller.set_window_icon = set_window_icon;
                caller.show_window = show_window;
                caller.hide_window = hide_window;
                caller.get_window_title = get_window_title;
                caller.set_window_title = set_window_title;
                caller.enable_fullscreen = enable_fullscreen;
                caller.disable_fullscreen = disable_fullscreen;
                caller.get_cursor_position = get_cursor_position;
                caller.set_cursor_position = set_cursor_position;
                caller.hide_cursor = hide_cursor;
                caller.show_cursor = show_cursor;
                caller.get_window_position = get_window_position;
                caller.set_window_position = set_window_position;
                caller.center_window = center_window;
                caller.update_resize_limit = update_resize_limit;
                caller.minimize_window = minimize_window;
                caller.maximize_window = maximize_window;
            }

            void init_ccall_data(LinuxCursorCaller &caller)
            {
                caller.create = create_cursor;
                caller.assign = assign_cursor;
                caller.destroy = destroy_cursor;
                caller.valid = is_cursor_valid;
            }
        } // namespace wayland
    } // namespace platform
} // namespace awin