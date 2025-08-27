#include <acul/log.hpp>
#include <awin/native_access.hpp>
#include <awin/window.hpp>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include "../env.hpp"
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

#define AWIN_BORDER_SIZE    4
#define AWIN_CAPTION_HEIGHT 24

namespace awin
{
    namespace platform
    {
        namespace wayland
        {

            static void assign_cursor(WaylandWindowData *window, Cursor::Platform *pd);

            static void pointer_handle_enter(void *user_data, wl_pointer *pointer, u32 serial, wl_surface *surface,
                                             wl_fixed_t sx, wl_fixed_t sy)
            {
                // Happens in the case we just destroyed the surface.
                if (!surface) return;
                if (wl_proxy_get_tag((wl_proxy *)surface) != &ctx.tag) return;

                WaylandWindowData *wl_data = (WaylandWindowData *)wl_surface_get_user_data(surface);

                ctx.serial = serial;
                ctx.pointer_enter_serial = serial;
                ctx.pointer_focus = wl_data;

                if (surface == wl_data->surface)
                {
                    wl_data->hovered = true;
                    if (wl_data->cursor) assign_cursor(wl_data, get_cursor_pd(wl_data->cursor));
                    acul::events::dispatch_event_group<MouseEnterEvent>(event_registry.mouse_enter, wl_data->owner,
                                                                        true);
                }
                else if (wl_data->fallback.decorations)
                    wl_data->fallback.focus = surface;
            }

            static void pointer_handle_leave(void *user_data, wl_pointer *pointer, u32 serial, wl_surface *surface)
            {
                if (!surface) return;
                if (wl_proxy_get_tag((struct wl_proxy *)surface) != &ctx.tag) return;

                auto *wl_data = ctx.pointer_focus;
                if (!wl_data) return;

                ctx.serial = serial;
                ctx.pointer_focus = NULL;
                ctx.cursor_previous_name = NULL;

                if (wl_data->hovered)
                {
                    wl_data->hovered = false;
                    acul::events::dispatch_event_group<MouseEnterEvent>(event_registry.mouse_enter, wl_data->owner,
                                                                        false);
                }
                else if (wl_data->fallback.decorations)
                    wl_data->fallback.focus = NULL;
            }

            static void pointer_handle_motion(void *user_data, wl_pointer *pointer, u32 time, wl_fixed_t sx,
                                              wl_fixed_t sy)
            {
                auto *wl_data = ctx.pointer_focus;
                if (!wl_data) return;

                wl_data->cursor_pos.x = wl_fixed_to_double(sx);
                wl_data->cursor_pos.y = wl_fixed_to_double(sy);
                acul::point2D<i32> cursor_pos = wl_data->cursor_pos;

                if (wl_data->hovered)
                {
                    ctx.cursor_previous_name = NULL;
                    acul::events::dispatch_event_group<PosEvent>(event_registry.mouse_move, event_id::mouse_move,
                                                                 wl_data->owner, cursor_pos);
                    return;
                }

                if (wl_data->fallback.decorations)
                {
                    const char *cursor_name = "left_ptr";

                    if (wl_data->flags & WindowFlagBits::resizable)
                    {
                        if (wl_data->fallback.focus == wl_data->fallback.top.surface)
                        {
                            if (cursor_pos.y < AWIN_BORDER_SIZE) cursor_name = "n-resize";
                        }
                        else if (wl_data->fallback.focus == wl_data->fallback.left.surface)
                            cursor_name = cursor_pos.y < AWIN_BORDER_SIZE ? "nw-resize" : "w-resize";
                        else if (wl_data->fallback.focus == wl_data->fallback.right.surface)
                            cursor_name = cursor_pos.y < AWIN_BORDER_SIZE ? "ne-resize" : "e-resize";
                        else if (wl_data->fallback.focus == wl_data->fallback.bottom.surface)
                        {
                            if (cursor_pos.x < AWIN_BORDER_SIZE)
                                cursor_name = "sw-resize";
                            else if (cursor_pos.x > wl_data->dimenstions.x + AWIN_BORDER_SIZE)
                                cursor_name = "se-resize";
                            else
                                cursor_name = "s-resize";
                        }
                    }

                    if (ctx.cursor_previous_name != cursor_name)
                    {
                        wl_surface *surface = ctx.cursor_surface;
                        wl_cursor_theme *theme = ctx.cursor_theme;
                        int scale = 1;

                        if (wl_data->buffer_scale > 1 && ctx.cursor_theme_hi_dpi)
                        {
                            // We only support up to scale=2 for now, since libwayland-cursor
                            // requires us to load a different theme for each size.
                            scale = 2;
                            theme = ctx.cursor_theme_hi_dpi;
                        }

                        wl_cursor *cursor = wl_cursor_theme_get_cursor(theme, cursor_name);
                        if (!cursor) return;

                        //         // TODO: handle animated cursors too.
                        wl_cursor_image *image = cursor->images[0];
                        if (!image) return;

                        wl_buffer *buffer = wl_cursor_image_get_buffer(image);
                        if (!buffer) return;

                        wl_pointer_set_cursor(ctx.pointer, ctx.pointer_enter_serial, surface, image->hotspot_x / scale,
                                              image->hotspot_y / scale);
                        wl_surface_set_buffer_scale(surface, scale);
                        wl_surface_attach(surface, buffer, 0, 0);
                        wl_surface_damage(surface, 0, 0, image->width, image->height);
                        wl_surface_commit(surface);

                        ctx.cursor_previous_name = cursor_name;
                    }
                }
            }

            static void pointer_handle_button(void *user_data, wl_pointer *pointer, u32 serial, u32 time, u32 button,
                                              u32 state)
            {
                auto *wl_data = ctx.pointer_focus;
                if (!wl_data) return;

                if (wl_data->hovered)
                {
                    ctx.serial = serial;
                    acul::events::dispatch_event_group<MouseClickEvent>(
                        event_registry.mouse_click, wl_data->owner, static_cast<io::MouseKey>(button - BTN_LEFT),
                        state == WL_POINTER_BUTTON_STATE_PRESSED ? io::KeyPressState::press
                                                                 : io::KeyPressState::release);
                    return;
                }

                if (wl_data->fallback.decorations)
                {
                    if (button == BTN_LEFT)
                    {
                        u32 edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;

                        if (wl_data->fallback.focus == wl_data->fallback.top.surface)
                        {
                            if (wl_data->cursor_pos.y < AWIN_BORDER_SIZE)
                                edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
                            else
                                xdg_toplevel_move(wl_data->xdg.toplevel, ctx.seat, serial);
                        }
                        else if (wl_data->fallback.focus == wl_data->fallback.left.surface)
                            edges = wl_data->cursor_pos.y < AWIN_BORDER_SIZE ? XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT
                                                                             : XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
                        else if (wl_data->fallback.focus == wl_data->fallback.right.surface)
                            edges = wl_data->cursor_pos.y < AWIN_BORDER_SIZE ? XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT
                                                                             : XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
                        else if (wl_data->fallback.focus == wl_data->fallback.bottom.surface)
                        {
                            if (wl_data->cursor_pos.x < AWIN_BORDER_SIZE)
                                edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                            else if (wl_data->cursor_pos.x > wl_data->dimenstions.x + AWIN_BORDER_SIZE)
                                edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                            else
                                edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
                        }
                        if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
                            xdg_toplevel_resize(wl_data->xdg.toplevel, ctx.seat, serial, edges);
                    }
                    else if (button == BTN_RIGHT && wl_data->xdg.toplevel)
                        xdg_toplevel_show_window_menu(wl_data->xdg.toplevel, ctx.seat, serial, wl_data->cursor_pos.x,
                                                      wl_data->cursor_pos.y);
                }
            }

            static void pointer_handle_axis(void *user_data, wl_pointer *pointer, u32 time, u32 axis, wl_fixed_t value)
            {
                auto *window_data = ctx.pointer_focus;
                if (!window_data) return;

                // NOTE: 10 units of motion per mouse wheel step seems to be a common ratio
                if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
                    acul::events::dispatch_event_group<ScrollEvent>(event_registry.scroll, window_data->owner,
                                                                    -wl_fixed_to_double(value) / 10.0, 0.0);
                else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
                    acul::events::dispatch_event_group<ScrollEvent>(event_registry.scroll, window_data->owner, 0.0,
                                                                    -wl_fixed_to_double(value) / 10.0);
            }

            static const struct wl_pointer_listener pointer_listener = {
                pointer_handle_enter,  pointer_handle_leave, pointer_handle_motion,
                pointer_handle_button, pointer_handle_axis,
            };

            static void keyboard_handle_keymap(void *user_data, wl_keyboard *keyboard, u32 format, int fd, u32 size)
            {
                xkb_keymap *keymap;
                xkb_state *state;
                xkb_compose_table *compose_table;
                xkb_compose_state *compose_state;

                char *map_str;
                const char *locale;

                if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
                {
                    close(fd);
                    return;
                }

                map_str = (char *)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
                if (map_str == MAP_FAILED)
                {
                    close(fd);
                    return;
                }

                keymap = xkb_keymap_new_from_string(ctx.xkb.context, map_str, XKB_KEYMAP_FORMAT_TEXT_V1,
                                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
                munmap(map_str, size);
                close(fd);

                if (!keymap)
                {
                    LOG_ERROR("Wayland: Failed to compile keymap");
                    return;
                }

                state = xkb_state_new(keymap);
                if (!state)
                {
                    LOG_ERROR("Wayland: Failed to create XKB state");
                    xkb_keymap_unref(keymap);
                    return;
                }

                // Look up the preferred locale, falling back to "C" as default.
                locale = getenv("LC_ALL");
                if (!locale) locale = getenv("LC_CTYPE");
                if (!locale) locale = getenv("LANG");
                if (!locale) locale = "C";

                compose_table =
                    xkb_compose_table_new_from_locale(ctx.xkb.context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
                if (compose_table)
                {
                    compose_state = xkb_compose_state_new(compose_table, XKB_COMPOSE_STATE_NO_FLAGS);
                    xkb_compose_table_unref(compose_table);
                    if (compose_state)
                        ctx.xkb.compose_state = compose_state;
                    else
                        LOG_ERROR("Wayland: Failed to create XKB compose state");
                }
                else
                    LOG_ERROR("Wayland: Failed to create XKB compose table");

                xkb_keymap_unref(ctx.xkb.keymap);
                xkb_state_unref(ctx.xkb.state);
                ctx.xkb.keymap = keymap;
                ctx.xkb.state = state;

                ctx.xkb.control_index = xkb_keymap_mod_get_index(ctx.xkb.keymap, "Control");
                ctx.xkb.alt_index = xkb_keymap_mod_get_index(ctx.xkb.keymap, "Mod1");
                ctx.xkb.shift_index = xkb_keymap_mod_get_index(ctx.xkb.keymap, "Shift");
                ctx.xkb.super_index = xkb_keymap_mod_get_index(ctx.xkb.keymap, "Mod4");
                ctx.xkb.caps_lock_index = xkb_keymap_mod_get_index(ctx.xkb.keymap, "Lock");
                ctx.xkb.num_lock_index = xkb_keymap_mod_get_index(ctx.xkb.keymap, "Mod2");
            }

            static void relative_pointer_handle_motion(void *user_data, zwp_relative_pointer_v1 *, u32 time_hi,
                                                       u32 time_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t,
                                                       wl_fixed_t)
            {
                WaylandWindowData *window = static_cast<WaylandWindowData *>(user_data);
                acul::point2D<i32> delta = {static_cast<i32>(wl_fixed_to_double(dx)),
                                            static_cast<i32>(wl_fixed_to_double(dy))};
                acul::events::dispatch_event_group<PosEvent>(event_registry.mouse_move_delta,
                                                             event_id::mouse_move_delta, window->owner, delta);
            }

            static const struct zwp_relative_pointer_v1_listener relative_pointer_listener = {
                .relative_motion = relative_pointer_handle_motion};

            void disable_relative_pointer(WaylandWindowData *window)
            {
                if (!window->relative_pointer) return;
                zwp_relative_pointer_v1_destroy(window->relative_pointer);
                window->relative_pointer = nullptr;
            }

            inline void mark_focus_window(WaylandWindowData *window)
            {
                window->focused = true;
                acul::events::dispatch_event_group<FocusEvent>(event_registry.focus, window->owner, true);
                if (!ctx.relative_pointer_manager || window->relative_pointer) return;
                window->relative_pointer =
                    zwp_relative_pointer_manager_v1_get_relative_pointer(ctx.relative_pointer_manager, ctx.pointer);
                zwp_relative_pointer_v1_add_listener(window->relative_pointer, &relative_pointer_listener, window);
            }

            inline void unmark_focus_window(WaylandWindowData *window)
            {
                window->focused = false;
                acul::events::dispatch_event_group<FocusEvent>(event_registry.focus, window->owner, false);
                if (!window->relative_pointer) return;
                zwp_relative_pointer_v1_destroy(window->relative_pointer);
                window->relative_pointer = nullptr;
            }

            static void keyboard_handle_enter(void *user_data, wl_keyboard *keyboard, u32 serial, wl_surface *surface,
                                              wl_array *keys)
            {
                // Happens in the case we just destroyed the surface.
                if (!surface || (wl_proxy_get_tag((wl_proxy *)surface) != &ctx.tag)) return;

                auto *wl_data = (WaylandWindowData *)wl_surface_get_user_data(surface);
                if (surface != wl_data->surface) return;

                ctx.serial = serial;
                ctx.keyboard_focus = wl_data;
                mark_focus_window(wl_data);
            }

            static void keyboard_handle_leave(void *user_data, wl_keyboard *keyboard, u32 serial, wl_surface *surface)
            {
                auto *window_data = ctx.keyboard_focus;
                if (!window_data) return;

                itimerspec timer = {{0}};
                timerfd_settime(ctx.key_repeat_timer_fd, 0, &timer, NULL);

                ctx.serial = serial;
                ctx.keyboard_focus = NULL;
                unmark_focus_window(window_data);
            }

            static xkb_keysym_t compose_symbol(xkb_keysym_t sym)
            {
                if (sym == XKB_KEY_NoSymbol || !ctx.xkb.compose_state) return sym;
                if (xkb_compose_state_feed(ctx.xkb.compose_state, sym) != XKB_COMPOSE_FEED_ACCEPTED) return sym;
                switch (xkb_compose_state_get_status(ctx.xkb.compose_state))
                {
                    case XKB_COMPOSE_COMPOSED:
                        return xkb_compose_state_get_one_sym(ctx.xkb.compose_state);
                    case XKB_COMPOSE_COMPOSING:
                    case XKB_COMPOSE_CANCELLED:
                        return XKB_KEY_NoSymbol;
                    case XKB_COMPOSE_NOTHING:
                    default:
                        return sym;
                }
            }

            static void input_text(WindowData *window_data, u32 scancode)
            {
                const xkb_keysym_t *keysyms;
                const xkb_keycode_t keycode = scancode + 8;

                if (xkb_state_key_get_syms(ctx.xkb.state, keycode, &keysyms) == 1)
                {
                    const xkb_keysym_t keysym = compose_symbol(keysyms[0]);
                    const u32 codepoint = xkb_keysym_to_utf32(keysym);
                    if (codepoint != 0)
                        acul::events::dispatch_event_group<CharInputEvent>(event_registry.char_input,
                                                                           window_data->owner, codepoint);
                }
            }

            static void keyboard_handle_key(void *user_data, wl_keyboard *keyboard, u32 serial, u32 time, u32 scancode,
                                            u32 state)
            {
                auto *window_data = ctx.keyboard_focus;
                if (!window_data) return;

                const io::KeyPressState action =
                    state == WL_KEYBOARD_KEY_STATE_PRESSED ? io::KeyPressState::press : io::KeyPressState::release;
                ctx.serial = serial;
                itimerspec timer = {{0}};

                if (action == io::KeyPressState::press)
                {
                    const xkb_keycode_t keycode = scancode + 8;

                    if (xkb_keymap_key_repeats(ctx.xkb.keymap, keycode) && ctx.key_repeat_rate > 0)
                    {
                        ctx.key_repeat_scancode = scancode;
                        if (ctx.key_repeat_rate > 1)
                            timer.it_interval.tv_nsec = 1000000000 / ctx.key_repeat_rate;
                        else
                            timer.it_interval.tv_sec = 1;

                        timer.it_value.tv_sec = ctx.key_repeat_delay / 1000;
                        timer.it_value.tv_nsec = (ctx.key_repeat_delay % 1000) * 1000000;
                    }
                }

                timerfd_settime(ctx.key_repeat_timer_fd, 0, &timer, NULL);
                const auto key = ctx.keymap.find(scancode);
                io::KeyMode mods = ctx.xkb.modifiers;
                if (key != io::Key::unknown) sync_mods_by_key(key, mods);
                input_key(window_data, key, action, mods);

                if (action == io::KeyPressState::press) input_text(window_data, scancode);
            }

            static void keyboard_handle_modifiers(void *user_data, wl_keyboard *keyboard, u32 serial,
                                                  u32 mods_depressed, u32 mods_latched, u32 mods_locked, u32 group)
            {
                ctx.serial = serial;

                if (!ctx.xkb.keymap) return;
                xkb_state_update_mask(ctx.xkb.state, mods_depressed, mods_latched, mods_locked, 0, 0, group);

                ctx.xkb.modifiers = {};

                struct
                {
                    xkb_mod_index_t index;
                    i8 bit;
                } modifiers[] = {
                    {ctx.xkb.control_index, io::KeyModeBits::control},
                    {ctx.xkb.alt_index, io::KeyModeBits::alt},
                    {ctx.xkb.shift_index, io::KeyModeBits::shift},
                    {ctx.xkb.super_index, io::KeyModeBits::super},
                    {ctx.xkb.caps_lock_index, io::KeyModeBits::caps_lock},
                    {ctx.xkb.num_lock_index, io::KeyModeBits::num_lock},
                };

                for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++)
                    if (xkb_state_mod_index_is_active(ctx.xkb.state, modifiers[i].index, XKB_STATE_MODS_EFFECTIVE) == 1)
                        ctx.xkb.modifiers |= modifiers[i].bit;
            }

            static void keyboard_handle_repeat_info(void *user_data, wl_keyboard *keyboard, i32 rate, i32 delay)
            {
                if (keyboard != ctx.keyboard) return;

                ctx.key_repeat_rate = rate;
                ctx.key_repeat_delay = delay;
            }

            static const struct wl_keyboard_listener keyboard_listener = {
                keyboard_handle_keymap, keyboard_handle_enter,     keyboard_handle_leave,
                keyboard_handle_key,    keyboard_handle_modifiers, keyboard_handle_repeat_info,
            };

            static void seat_handle_capabilities(void *user_data, wl_seat *seat, u32 caps)
            {
                if ((caps & WL_SEAT_CAPABILITY_POINTER) && !ctx.pointer)
                {
                    ctx.pointer = wl_seat_get_pointer(seat);
                    wl_pointer_add_listener(ctx.pointer, &pointer_listener, NULL);
                }
                else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && ctx.pointer)
                {
                    wl_pointer_destroy(ctx.pointer);
                    ctx.pointer = NULL;
                }

                if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !ctx.keyboard)
                {
                    ctx.keyboard = wl_seat_get_keyboard(seat);
                    wl_keyboard_add_listener(ctx.keyboard, &keyboard_listener, NULL);
                }
                else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && ctx.keyboard)
                {
                    wl_keyboard_destroy(ctx.keyboard);
                    ctx.keyboard = NULL;
                }
            }

            static void seat_handle_name(void *user_data, wl_seat *seat, const char *name) {}

            static const struct wl_seat_listener seat_listener = {
                seat_handle_capabilities,
                seat_handle_name,
            };

            void add_seat_listener(wl_seat *seat) { wl_seat_add_listener(seat, &seat_listener, NULL); }

            static void data_offer_handle_offer(void *user_data, wl_data_offer *offer, const char *mime_type)
            {
                for (auto &info : ctx.offers)
                {
                    if (info.offer != offer) continue;

                    if (strcmp(mime_type, "text/plain;charset=utf-8") == 0)
                        info.text_plain_utf8 = true;
                    else if (strcmp(mime_type, "text/uri-list") == 0)
                        info.text_uri_list = true;
                    break;
                }
            }

            static const struct wl_data_offer_listener data_offer_listener = {data_offer_handle_offer};

            static void data_device_handle_data_offer(void *user_data, wl_data_device *device, wl_data_offer *offer)
            {
                ctx.offers.emplace_back(offer);
                wl_data_offer_add_listener(offer, &data_offer_listener, nullptr);
            }

            static void data_device_handle_enter(void *user_data, wl_data_device *device, u32 serial,
                                                 wl_surface *surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer *offer)
            {
            }

            static void data_device_handle_leave(void *user_data, wl_data_device *device) {}

            static void data_device_handle_motion(void *user_data, wl_data_device *device, u32 time, wl_fixed_t x,
                                                  wl_fixed_t y)
            {
            }

            static void data_device_handle_drop(void *user_data, wl_data_device *device) {}

            static void data_device_handle_selection(void *user_data, wl_data_device *device, wl_data_offer *offer)
            {
                if (ctx.selection_offer)
                {
                    wl_data_offer_destroy(ctx.selection_offer);
                    ctx.selection_offer = NULL;
                }

                for (unsigned int i = 0; i < ctx.offers.size(); i++)
                {
                    if (ctx.offers[i].offer == offer)
                    {
                        if (ctx.offers[i].text_plain_utf8)
                            ctx.selection_offer = offer;
                        else
                            wl_data_offer_destroy(offer);

                        ctx.offers[i] = ctx.offers.back();
                        ctx.offers.pop_back();
                        break;
                    }
                }
            }

            const struct wl_data_device_listener data_device_listener = {
                data_device_handle_data_offer, data_device_handle_enter, data_device_handle_leave,
                data_device_handle_motion,     data_device_handle_drop,  data_device_handle_selection,
            };

            void add_data_device_listener(wl_data_device *device)
            {
                wl_data_device_add_listener(device, &data_device_listener, NULL);
            }

            void fractional_scale_handle_preferred_scale(void *user_data, wp_fractional_scale_v1 *fractional_scale,
                                                         u32 numerator)
            {
                auto *window = (WaylandWindowData *)user_data;

                window->scaling_numerator = numerator;
                const f32 dpi = numerator / 120.f;
                acul::events::dispatch_event_group<DpiChangedEvent>(event_registry.dpi_changed, window->owner, dpi,
                                                                    dpi);
            }

            const struct wp_fractional_scale_v1_listener fractional_scale_listener = {
                fractional_scale_handle_preferred_scale,
            };

            static void xdg_top_level_handle_configure(void *user_data, xdg_toplevel *toplevel, i32 width, i32 height,
                                                       wl_array *states)
            {
                auto *window = (WaylandWindowData *)user_data;

                window->pending = {};

                auto *data = static_cast<u32 *>(states->data);
                size_t count = states->size / sizeof(u32);

                for (size_t i = 0; i < count; ++i)
                {
                    u32 state = data[i];
                    switch (state)
                    {
                        case XDG_TOPLEVEL_STATE_MAXIMIZED:
                            window->pending.flags |= WindowFlagBits::maximized;
                            break;
                        case XDG_TOPLEVEL_STATE_FULLSCREEN:
                            window->pending.flags |= WindowFlagBits::fullscreen;
                            break;
                        case XDG_TOPLEVEL_STATE_ACTIVATED:
                            window->pending.flags |= WindowFlagBits::activated;
                            break;
                        default:
                            break;
                    }
                }

                if (width && height)
                {
                    if (window->fallback.decorations)
                    {
                        window->pending.dimensions.x = std::max(0, width - AWIN_BORDER_SIZE * 2);
                        window->pending.dimensions.y = std::max(0, height - AWIN_BORDER_SIZE - AWIN_CAPTION_HEIGHT);
                    }
                    else
                        window->pending.dimensions = {width, height};
                }
                else
                    window->pending.dimensions = window->dimenstions;
            }

            static void xdg_toplevel_handle_close(void *user_data, xdg_toplevel *toplevel)
            {
                auto *window = (WaylandWindowData *)user_data;
                window->ready_to_close = true;
            }

            static const struct xdg_toplevel_listener xdg_toplevel_listener = {xdg_top_level_handle_configure,
                                                                               xdg_toplevel_handle_close};

            static bool resize_window(WaylandWindowData *window, acul::point2D<int> dimensions)
            {
                dimensions.x = std::max(dimensions.x, 1);
                dimensions.y = std::max(dimensions.y, 1);

                if (dimensions.x == window->dimenstions.x && dimensions.y == window->dimenstions.y) return false;
                window->dimenstions = dimensions;

                if (window->scaling_viewport)
                    wp_viewport_set_destination(window->scaling_viewport, window->dimenstions.x, window->dimenstions.y);

                if (window->fallback.decorations)
                {
                    wp_viewport_set_destination(window->fallback.top.viewport, window->dimenstions.x,
                                                AWIN_CAPTION_HEIGHT);
                    wl_surface_commit(window->fallback.top.surface);
                    wp_viewport_set_destination(window->fallback.left.viewport, AWIN_BORDER_SIZE,
                                                window->dimenstions.y + AWIN_CAPTION_HEIGHT);
                    wl_surface_commit(window->fallback.left.surface);
                    wl_subsurface_set_position(window->fallback.right.subsurface, window->dimenstions.x,
                                               -AWIN_CAPTION_HEIGHT);
                    wp_viewport_set_destination(window->fallback.right.viewport, AWIN_BORDER_SIZE,
                                                window->dimenstions.y + AWIN_CAPTION_HEIGHT);
                    wl_surface_commit(window->fallback.right.surface);
                    wl_subsurface_set_position(window->fallback.bottom.subsurface, -AWIN_BORDER_SIZE,
                                               window->dimenstions.y);
                    wp_viewport_set_destination(window->fallback.bottom.viewport,
                                                window->dimenstions.x + AWIN_BORDER_SIZE * 2, AWIN_BORDER_SIZE);
                    wl_surface_commit(window->fallback.bottom.surface);
                }
                return true;
            }

            static void xdg_surface_handle_configure(void *user_data, xdg_surface *surface, u32 serial)
            {
                auto *window = (WaylandWindowData *)user_data;
                xdg_surface_ack_configure(surface, serial);

                const bool is_pending_activated = window->pending.flags & WindowFlagBits::activated;
                if (window->activated != is_pending_activated)
                {
                    window->activated = is_pending_activated;
                    if (!window->activated) xdg_toplevel_set_minimized(window->xdg.toplevel);
                }

                const bool is_pending_maximized = window->pending.flags & WindowFlagBits::maximized;
                const bool is_maximized = window->flags & WindowFlagBits::maximized;
                if (is_maximized != is_pending_maximized)
                {
                    window->flags = is_pending_maximized ? (window->flags | WindowFlagBits::maximized)
                                                         : (window->flags & ~WindowFlagBits::maximized);
                    acul::events::dispatch_event_group<StateEvent>(event_registry.maximize, event_id::maximize,
                                                                   window->owner, is_pending_maximized);
                }
                const bool is_pending_fullscreen = window->pending.flags & WindowFlagBits::fullscreen;
                window->flags = is_pending_fullscreen ? (window->flags | WindowFlagBits::fullscreen)
                                                      : (window->flags & ~WindowFlagBits::fullscreen);

                if (resize_window(window, window->pending.dimensions))
                    acul::events::dispatch_event_group<PosEvent>(event_registry.resize, event_id::resize, window->owner,
                                                                 window->pending.dimensions);
            }

            static const struct xdg_surface_listener xdg_surface_listener = {xdg_surface_handle_configure};

            void libdecor_frame_handle_configure(libdecor_frame *frame, libdecor_configuration *config, void *user_data)
            {
                WaylandWindowData *window = (WaylandWindowData *)user_data;
                acul::point2D<int> size;

                enum libdecor_window_state window_state;
                bool fullscreen, activated, maximized;

                if (libdecor_configuration_get_window_state(config, &window_state))
                {
                    fullscreen = (window_state & LIBDECOR_WINDOW_STATE_FULLSCREEN) != 0;
                    activated = (window_state & LIBDECOR_WINDOW_STATE_ACTIVE) != 0;
                    maximized = (window_state & LIBDECOR_WINDOW_STATE_MAXIMIZED) != 0;
                }
                else
                {
                    fullscreen = window->flags & WindowFlagBits::fullscreen;
                    activated = window->activated;
                    maximized = window->flags & WindowFlagBits::maximized;
                }

                if (!libdecor_configuration_get_content_size(config, frame, &size.x, &size.y))
                    size = window->dimenstions;

                libdecor_state *frame_state = libdecor_state_new(size.x, size.y);
                libdecor_frame_commit(frame, frame_state, config);
                libdecor_state_free(frame_state);

                window->activated = activated;

                if ((window->flags & WindowFlagBits::maximized) != maximized)
                {
                    window->flags = maximized ? (window->flags | WindowFlagBits::maximized)
                                              : (window->flags & ~WindowFlagBits::maximized);
                    acul::events::dispatch_event_group<StateEvent>(event_registry.maximize, event_id::maximize,
                                                                   window->owner, maximized);
                }

                window->flags = fullscreen ? (window->flags | WindowFlagBits::fullscreen)
                                           : (window->flags & ~WindowFlagBits::fullscreen);

                if (!(window->flags & WindowFlagBits::hidden)) window->flags &= ~WindowFlagBits::hidden;

                if (resize_window(window, size))
                    acul::events::dispatch_event_group<PosEvent>(event_registry.resize, event_id::resize, window->owner,
                                                                 window->dimenstions);
                wl_surface_commit(window->surface);
            }

            void libdecor_frame_handle_close(libdecor_frame *frame, void *user_data)
            {
                WaylandWindowData *window = (WaylandWindowData *)user_data;
                window->ready_to_close = true;
            }

            void libdecor_frame_handle_commit(libdecor_frame *frame, void *user_data)
            {
                WaylandWindowData *window = (WaylandWindowData *)user_data;
                wl_surface_commit(window->surface);
            }

            void libdecor_frame_handle_dismiss_popup(libdecor_frame *frame, const char *seat_name, void *user_data) {}

            static const struct libdecor_frame_interface libdecor_frame_interface = {
                libdecor_frame_handle_configure, libdecor_frame_handle_close, libdecor_frame_handle_commit,
                libdecor_frame_handle_dismiss_popup};

            static void set_idle_inhibitor(WaylandWindowData *wl_data, bool enable)
            {
                if (enable && !wl_data->idle_inhibitor && ctx.idle_inhibit_manager)
                {
                    wl_data->idle_inhibitor =
                        zwp_idle_inhibit_manager_v1_create_inhibitor(ctx.idle_inhibit_manager, wl_data->surface);
                    if (!wl_data->idle_inhibitor) LOG_ERROR("Wayland: Failed to create idle inhibitor");
                }
                else if (!enable && wl_data->idle_inhibitor)
                {
                    zwp_idle_inhibitor_v1_destroy(wl_data->idle_inhibitor);
                    wl_data->idle_inhibitor = NULL;
                }
            }

            static bool create_libdecor_frame(WaylandWindowData *window)
            {
                // Allow libdecor to finish initialization of itself and its plugin
                while (!ctx.libdecor.ready) wait_events();

                window->libdecor_frame =
                    libdecor_decorate(ctx.libdecor.context, window->surface, &libdecor_frame_interface, window);
                if (!window->libdecor_frame)
                {
                    LOG_ERROR("Wayland: Failed to create libdecor frame");
                    return false;
                }

                libdecor_state *frame_state = libdecor_state_new(window->dimenstions.x, window->dimenstions.y);
                libdecor_frame_commit(window->libdecor_frame, frame_state, NULL);
                libdecor_state_free(frame_state);

                libdecor_frame_set_app_id(window->libdecor_frame, window->title.c_str());
                libdecor_frame_set_title(window->libdecor_frame, window->title.c_str());

                if (window->resize_limit.x != 0 && window->resize_limit.y != 0)
                    libdecor_frame_set_min_content_size(window->libdecor_frame, window->resize_limit.x,
                                                        window->resize_limit.y);

                if (!(window->flags & WindowFlagBits::resizable))
                    libdecor_frame_unset_capabilities(window->libdecor_frame, LIBDECOR_ACTION_RESIZE);

                if (window->flags & WindowFlagBits::fullscreen)
                {
                    libdecor_frame_set_fullscreen(window->libdecor_frame, NULL);
                    set_idle_inhibitor(window, true);
                }
                else
                {
                    if (window->flags & WindowFlagBits::maximized) libdecor_frame_set_maximized(window->libdecor_frame);
                    if (!(window->flags & WindowFlagBits::decorated))
                        libdecor_frame_set_visibility(window->libdecor_frame, false);
                    set_idle_inhibitor(window, false);
                }

                libdecor_frame_map(window->libdecor_frame);
                wl_display_roundtrip(ctx.display);
                return true;
            }

            static void update_xdg_size_limits(WaylandWindowData *window)
            {
                acul::point2D<int> limit;
                if (window->flags & WindowFlagBits::resizable)
                {
                    if (window->resize_limit.x == 0 || window->resize_limit.y == 0)
                        limit = {0, 0};
                    else
                    {
                        limit = window->resize_limit;
                        if (window->fallback.decorations)
                            limit += {AWIN_BORDER_SIZE * 2, AWIN_CAPTION_HEIGHT + AWIN_BORDER_SIZE};
                    }
                }
                else
                    limit = window->dimenstions;

                xdg_toplevel_set_min_size(window->xdg.toplevel, limit.x, limit.y);
            }

            static void create_fallback_edge(WaylandWindowData *window, FallbackEdgeWayland *edge, wl_surface *parent,
                                             wl_buffer *buffer, acul::point2D<int> pos, acul::point2D<int> size)
            {
                edge->surface = wl_compositor_create_surface(ctx.compositor);
                wl_surface_set_user_data(edge->surface, window);
                wl_proxy_set_tag((struct wl_proxy *)edge->surface, &ctx.tag);
                edge->subsurface = wl_subcompositor_get_subsurface(ctx.subcompositor, edge->surface, parent);
                wl_subsurface_set_position(edge->subsurface, pos.x, pos.y);
                edge->viewport = wp_viewporter_get_viewport(ctx.viewporter, edge->surface);
                wp_viewport_set_destination(edge->viewport, size.x, size.y);
                wl_surface_attach(edge->surface, buffer, 0, 0);

                wl_region *region = wl_compositor_create_region(ctx.compositor);
                wl_region_add(region, 0, 0, size.x, size.y);
                wl_surface_set_opaque_region(edge->surface, region);
                wl_surface_commit(edge->surface);
                wl_region_destroy(region);
            }

            static int create_tmpfile_cloexec(char *tmpname)
            {
                int fd;

                fd = mkostemp(tmpname, O_CLOEXEC);
                if (fd >= 0) unlink(tmpname);

                return fd;
            }

            static int create_anonymous_file(off_t size)
            {
                static acul::string _template = "/awin-shared-XXXXXX";
                const char *path;
                int fd;
                int ret;

                fd = memfd_create("awin-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
                if (fd >= 0)
                    fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
                else
                {
                    path = getenv("XDG_RUNTIME_DIR");
                    if (!path)
                    {
                        errno = ENOENT;
                        return -1;
                    }

                    acul::string name{path};
                    name += _template;

                    fd = create_tmpfile_cloexec((char *)name.c_str());
                    if (fd < 0) return -1;
                }

                ret = posix_fallocate(fd, 0, size);
                if (ret != 0)
                {
                    close(fd);
                    errno = ret;
                    return -1;
                }
                return fd;
            }

            static wl_buffer *create_shm_buffer(const Image *image)
            {
                const int stride = image->dimenstions.x * 4;
                const int length = image->dimenstions.x * image->dimenstions.y * 4;

                const int fd = create_anonymous_file(length);
                if (fd < 0)
                {
                    LOG_ERROR("Wayland: Failed to create buffer file of size %d: %s", length, strerror(errno));
                    return NULL;
                }

                void *data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                if (data == MAP_FAILED)
                {
                    LOG_ERROR("Wayland: Failed to map file: %s", strerror(errno));
                    close(fd);
                    return NULL;
                }

                struct wl_shm_pool *pool = wl_shm_create_pool(ctx.shm, fd, length);

                close(fd);

                unsigned char *source = (unsigned char *)image->pixels;
                unsigned char *target = (unsigned char *)data;
                for (int i = 0; i < image->dimenstions.x * image->dimenstions.y; i++, source += 4)
                {
                    unsigned int alpha = source[3];

                    *target++ = (unsigned char)((source[2] * alpha) / 255);
                    *target++ = (unsigned char)((source[1] * alpha) / 255);
                    *target++ = (unsigned char)((source[0] * alpha) / 255);
                    *target++ = (unsigned char)alpha;
                }

                wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, image->dimenstions.x, image->dimenstions.y,
                                                              stride, WL_SHM_FORMAT_ARGB8888);
                munmap(data, length);
                wl_shm_pool_destroy(pool);

                return buffer;
            }

            static void create_fallback_decorations(WaylandWindowData *window)
            {
                unsigned char data[] = {224, 224, 224, 255};
                const Image image = {{1, 1}, data};

                if (!ctx.viewporter) return;

                if (!window->fallback.buffer) window->fallback.buffer = create_shm_buffer(&image);
                if (!window->fallback.buffer) return;

                create_fallback_edge(window, &window->fallback.top, window->surface, window->fallback.buffer,
                                     {0, -AWIN_CAPTION_HEIGHT}, {window->dimenstions.x, AWIN_CAPTION_HEIGHT});
                create_fallback_edge(window, &window->fallback.left, window->surface, window->fallback.buffer,
                                     {-AWIN_BORDER_SIZE, -AWIN_CAPTION_HEIGHT},
                                     {AWIN_BORDER_SIZE, window->dimenstions.y + AWIN_CAPTION_HEIGHT});
                create_fallback_edge(window, &window->fallback.right, window->surface, window->fallback.buffer,
                                     {window->dimenstions.x, -AWIN_CAPTION_HEIGHT},
                                     {AWIN_BORDER_SIZE, window->dimenstions.y + AWIN_CAPTION_HEIGHT});
                create_fallback_edge(window, &window->fallback.bottom, window->surface, window->fallback.buffer,
                                     {-AWIN_BORDER_SIZE, window->dimenstions.y},
                                     {window->dimenstions.y + AWIN_BORDER_SIZE * 2, AWIN_BORDER_SIZE});

                window->fallback.decorations = true;
            }

            static void destroy_fallback_edge(FallbackEdgeWayland *edge)
            {
                if (edge->subsurface) wl_subsurface_destroy(edge->subsurface);
                if (edge->surface) wl_surface_destroy(edge->surface);
                if (edge->viewport) wp_viewport_destroy(edge->viewport);

                edge->surface = NULL;
                edge->subsurface = NULL;
                edge->viewport = NULL;
            }

            static void destroy_fallback_decorations(WaylandWindowData *window)
            {
                window->fallback.decorations = false;
                destroy_fallback_edge(&window->fallback.top);
                destroy_fallback_edge(&window->fallback.left);
                destroy_fallback_edge(&window->fallback.right);
                destroy_fallback_edge(&window->fallback.bottom);
            }

            static void xdg_decoration_handle_configure(void *user_data, zxdg_toplevel_decoration_v1 *decoration,
                                                        u32 mode)
            {
                WaylandWindowData *window = (WaylandWindowData *)user_data;
                window->xdg.decoration_mode = mode;
                if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE)
                {
                    if (window->flags & WindowFlagBits::decorated) create_fallback_decorations(window);
                }
                else
                    destroy_fallback_decorations(window);
            }

            static const struct zxdg_toplevel_decoration_v1_listener xdg_decoration_listener = {
                xdg_decoration_handle_configure,
            };

            static bool create_shell_objects(WaylandWindowData *wl_data)
            {
                mark_focus_window(wl_data);
                if (ctx.libdecor.context)
                {
                    if (create_libdecor_frame(wl_data))
                    {
#ifdef AWIN_TEST_BUILD
                        if (ctx.is_surface_placeholder_enabled)
                        {
                            static wl_buffer *stub = nullptr;
                            if (!stub)
                            {
                                unsigned char data[] = {0, 0, 0, 255};
                                const Image image = {{1, 1}, data};
                                stub = create_shm_buffer(&image);
                            }
                            wl_surface_attach(wl_data->surface, stub, 0, 0);
                            wl_surface_damage(wl_data->surface, 0, 0, 1, 1);
                            wl_surface_commit(wl_data->surface);
                        }
#endif
                        return true;
                    }
                }
                wl_data->xdg.surface = xdg_wm_base_get_xdg_surface(ctx.wm_base, wl_data->surface);
                if (!wl_data->surface)
                {
                    LOG_ERROR("Wayland: Failed to create xdg-surface for window");
                    return false;
                }

                xdg_surface_add_listener(wl_data->xdg.surface, &xdg_surface_listener, wl_data);

                wl_data->xdg.toplevel = xdg_surface_get_toplevel(wl_data->xdg.surface);
                if (!wl_data->xdg.toplevel)
                {
                    LOG_ERROR("Wayland: Failed to create xdg-toplevel for window");
                    return false;
                }

                xdg_toplevel_add_listener(wl_data->xdg.toplevel, &xdg_toplevel_listener, wl_data);

                xdg_toplevel_set_app_id(wl_data->xdg.toplevel, wl_data->title.c_str());
                xdg_toplevel_set_title(wl_data->xdg.toplevel, wl_data->title.c_str());

                if (wl_data->flags & WindowFlagBits::fullscreen)
                {
                    xdg_toplevel_set_fullscreen(wl_data->xdg.toplevel, NULL);
                    set_idle_inhibitor(wl_data, true);
                }
                else
                {
                    if (wl_data->flags & WindowFlagBits::maximized) xdg_toplevel_set_maximized(wl_data->xdg.toplevel);
                    set_idle_inhibitor(wl_data, false);
                }

                if (ctx.decoration_manager)
                {
                    wl_data->xdg.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(ctx.decoration_manager,
                                                                                                 wl_data->xdg.toplevel);
                    zxdg_toplevel_decoration_v1_add_listener(wl_data->xdg.decoration, &xdg_decoration_listener,
                                                             wl_data);

                    u32 mode = wl_data->flags & WindowFlagBits::decorated
                                   ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
                                   : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
                    zxdg_toplevel_decoration_v1_set_mode(wl_data->xdg.decoration, mode);
                }
                else if (wl_data->flags & WindowFlagBits::decorated)
                    create_fallback_decorations(wl_data);

                update_xdg_size_limits(wl_data);

                wl_surface_commit(wl_data->surface);
                wl_display_roundtrip(ctx.display);
                return true;
            }

            static void surface_handle_enter(void *user_data, wl_surface *surface, wl_output *output)
            {
                if (wl_proxy_get_tag((struct wl_proxy *)output) != &ctx.tag) return;

                WaylandWindowData *window = (WaylandWindowData *)user_data;
                Output *_output = (Output *)wl_output_get_user_data(output);
                window->output = _output;
                if (!window || !_output) return;
                _output->windows.push_back(window);
                window->output_scales.emplace_back(output, _output->scale);
                update_buffer_scale_from_outputs(window);
            }

            static void surface_handle_leave(void *user_data, wl_surface *surface, wl_output *output)
            {
                if (wl_proxy_get_tag((wl_proxy *)output) != &ctx.tag) return;

                WaylandWindowData *window = (WaylandWindowData *)user_data;
                Output *_output = (Output *)wl_output_get_user_data(output);
                if (!window || !_output) return;
                auto itw = std::find(_output->windows.begin(), _output->windows.end(), window);
                if (itw != _output->windows.end()) _output->windows.erase(itw);

                if (window->output == _output) window->output = nullptr;

                auto it = std::find_if(window->output_scales.begin(), window->output_scales.end(),
                                       [output](auto &s) { return s.output == output; });
                if (it != window->output_scales.end())
                {
                    *it = window->output_scales.back();
                    window->output_scales.pop_back();
                }

                update_buffer_scale_from_outputs(window);
            }

            static const struct wl_surface_listener surface_listener = {surface_handle_enter, surface_handle_leave};

            bool create_window(WindowData *window_data, const acul::string &title, i32 width, i32 height,
                               WindowFlags flags)
            {
                auto *wl_data = (WaylandWindowData *)window_data;
                wl_data->surface = wl_compositor_create_surface(ctx.compositor);
                LOG_INFO("Wayland: Created window surface: %p", wl_data->surface);
                if (!wl_data->surface)
                {
                    LOG_ERROR("Wayland: Failed to create window surface");
                    return false;
                }
                wl_proxy_set_tag((wl_proxy *)wl_data->surface, &ctx.tag);
                wl_surface_add_listener(wl_data->surface, &surface_listener, wl_data);
                wl_data->dimenstions.x = width;
                wl_data->dimenstions.y = height;
                wl_data->flags = flags;
                wl_data->buffer_scale = 1;
                wl_data->scaling_numerator = 120;
                wl_data->scale_framebuffer = true;

                if (ctx.fractional_scale_manager && wl_data->scale_framebuffer)
                {
                    wl_data->scaling_viewport = wp_viewporter_get_viewport(ctx.viewporter, wl_data->surface);
                    wp_viewport_set_destination(wl_data->scaling_viewport, wl_data->dimenstions.x,
                                                wl_data->dimenstions.y);
                    wl_data->fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale(
                        ctx.fractional_scale_manager, wl_data->surface);
                    wp_fractional_scale_v1_add_listener(wl_data->fractional_scale, &fractional_scale_listener, wl_data);
                }

                wl_data->title = title;
                if (!(flags & WindowFlagBits::hidden))
                {
                    if (!create_shell_objects(wl_data)) return false;
                }
                wl_data->cursor = &env.default_cursor;
                return true;
            }

            static void destroy_shell_objects(WaylandWindowData *window)
            {
                destroy_fallback_decorations(window);
                if (window->libdecor_frame) libdecor_frame_unref(window->libdecor_frame);
                if (window->xdg.decoration) zxdg_toplevel_decoration_v1_destroy(window->xdg.decoration);
                if (window->xdg.toplevel) xdg_toplevel_destroy(window->xdg.toplevel);
                if (window->xdg.surface) xdg_surface_destroy(window->xdg.surface);

                window->libdecor_frame = NULL;
                window->xdg.decoration = NULL;
                window->xdg.decoration_mode = 0;
                window->xdg.toplevel = NULL;
                window->xdg.surface = NULL;
            }

            void destroy(WindowData *window_data)
            {
                auto *wl_data = (WaylandWindowData *)window_data;
                if (wl_data == ctx.pointer_focus) ctx.pointer_focus = NULL;
                if (wl_data == ctx.keyboard_focus) ctx.keyboard_focus = NULL;
                if (wl_data->fractional_scale) wp_fractional_scale_v1_destroy(wl_data->fractional_scale);
                if (wl_data->scaling_viewport) wp_viewport_destroy(wl_data->scaling_viewport);
                if (wl_data->idle_inhibitor) zwp_idle_inhibitor_v1_destroy(wl_data->idle_inhibitor);
                destroy_shell_objects(wl_data);
                if (wl_data->fallback.buffer) wl_buffer_destroy(wl_data->fallback.buffer);
                LOG_INFO("Wayland: Destroying window surface: %p", wl_data->surface);
                if (wl_data->surface) wl_surface_destroy(wl_data->surface);
                wl_data->output_scales.clear();
            }

            static bool flush_display()
            {
                while (wl_display_flush(ctx.display) == -1)
                {
                    if (errno != EAGAIN) return false;

                    pollfd fd = {wl_display_get_fd(ctx.display), POLLOUT};

                    while (poll(&fd, 1, -1) == -1)
                        if (errno != EINTR && errno != EAGAIN) return false;
                }

                return true;
            }

            static void set_cursor_image(WaylandWindowData *wl_data, WaylandCursor *wl_cursor)
            {
                itimerspec timer = {{0}};
                auto *handle = wl_cursor->handle;
                wl_cursor_image *image;
                wl_buffer *buffer;
                wl_surface *surface = ctx.cursor_surface;
                int scale = 1;

                if (!handle)
                    buffer = wl_cursor->buffer;
                else
                {
                    if (wl_data->buffer_scale > 1) scale = 2;

                    image = handle->images[wl_cursor->current_image];
                    buffer = wl_cursor_image_get_buffer(image);
                    if (!buffer) return;

                    timer.it_value.tv_sec = image->delay / 1000;
                    timer.it_value.tv_nsec = (image->delay % 1000) * 1000000;
                    timerfd_settime(ctx.cursor_timer_fd, 0, &timer, NULL);

                    wl_cursor->size.x = image->width;
                    wl_cursor->size.y = image->height;
                    wl_cursor->hot.x = image->hotspot_x;
                    wl_cursor->hot.y = image->hotspot_y;
                }

                wl_pointer_set_cursor(ctx.pointer, ctx.pointer_enter_serial, surface, wl_cursor->hot.x / scale,
                                      wl_cursor->hot.y / scale);
                wl_surface_set_buffer_scale(surface, scale);
                wl_surface_attach(surface, buffer, 0, 0);
                wl_surface_damage(surface, 0, 0, wl_cursor->size.x, wl_cursor->size.y);
                wl_surface_commit(surface);
            }

            static void increment_cursor_image(WaylandWindowData *wl_data)
            {
                WaylandCursor *cursor;

                if (!wl_data || !wl_data->hovered) return;

                cursor = (WaylandCursor *)get_cursor_pd(wl_data->cursor);
                if (cursor && cursor->handle)
                {
                    ++cursor->current_image;
                    cursor->current_image %= cursor->handle->image_count;
                    set_cursor_image(wl_data, cursor);
                }
            }

            static void handle_events(f64 *timeout)
            {
                bool event = false;
                enum
                {
                    DISPLAY_FD,
                    KEYREPEAT_FD,
                    CURSOR_FD,
                    LIBDECOR_FD
                };
                pollfd fds[] = {{wl_display_get_fd(ctx.display), POLLIN},
                                {ctx.key_repeat_timer_fd, POLLIN},
                                {ctx.cursor_timer_fd, POLLIN},
                                {-1, POLLIN}};

                if (ctx.libdecor.context) fds[LIBDECOR_FD].fd = libdecor_get_fd(ctx.libdecor.context);

                while (!event)
                {
                    while (wl_display_prepare_read(ctx.display) != 0)
                        if (wl_display_dispatch_pending(ctx.display) > 0) return;

                    // If an error other than EAGAIN happens, we have likely been disconnected
                    // from the Wayland session; try to handle that the best we can.
                    if (!flush_display())
                    {
                        wl_display_cancel_read(ctx.display);

                        for (const auto &output : ctx.outputs)
                            for (auto *window : output.windows) window->ready_to_close = true;
                        return;
                    }

                    if (!poll_posix(fds, sizeof(fds) / sizeof(fds[0]), timeout))
                    {
                        wl_display_cancel_read(ctx.display);
                        return;
                    }

                    if (fds[DISPLAY_FD].revents & POLLIN)
                    {
                        wl_display_read_events(ctx.display);
                        if (wl_display_dispatch_pending(ctx.display) > 0) event = true;
                    }
                    else
                        wl_display_cancel_read(ctx.display);

                    if (fds[KEYREPEAT_FD].revents & POLLIN)
                    {
                        u64 repeats;

                        if (read(ctx.key_repeat_timer_fd, &repeats, sizeof(repeats)) == 8)
                        {
                            if (ctx.keyboard_focus)
                            {
                                const auto key = ctx.keymap.find(ctx.key_repeat_scancode);
                                for (u64 i = 0; i < repeats; i++)
                                {
                                    input_key(ctx.keyboard_focus, key, io::KeyPressState::press, ctx.xkb.modifiers);
                                    input_text(ctx.keyboard_focus, ctx.key_repeat_scancode);
                                }

                                event = true;
                            }
                        }
                    }

                    if (fds[CURSOR_FD].revents & POLLIN)
                    {
                        u64 repeats;
                        if (read(ctx.cursor_timer_fd, &repeats, sizeof(repeats)) == 8)
                            increment_cursor_image(ctx.pointer_focus);
                    }
                }

                if (fds[LIBDECOR_FD].revents & POLLIN)
                {
                    if (libdecor_dispatch(ctx.libdecor.context, 0) > 0) event = true;
                }
            }

            void poll_events()
            {
                f64 timeout = 0.0;
                handle_events(&timeout);
            }

            void wait_events() { handle_events(NULL); }

            void wait_events_timeout() { handle_events(env.timeout > WINDOW_TIMEOUT_INF ? &env.timeout : NULL); }

            void push_empty_event()
            {
                wl_display_sync(ctx.display);
                flush_display();
            }

            f32 get_dpi(WindowData *window_data)
            {
                auto *wl_data = (WaylandWindowData *)window_data;
                auto *output = wl_data->output;
                if (!output) return 1.0f;
                return ctx.fractional_scale_manager ? output->scale : output->scale / 100.0f;
            }

            acul::point2D<i32> get_window_size(const Window &window)
            {
                auto *window_data = get_window_data(window);
                return window_data->dimenstions;
            }

            static void data_source_handle_target(void *user_data, wl_data_source *source, const char *mime_type)
            {
                if (ctx.selection_source != source)
                {
                    LOG_ERROR("Wayland: Unknown clipboard data source");
                    return;
                }
            }

            static void data_source_handle_send(void *user_data, wl_data_source *source, const char *mime_type, int fd)
            {
                // Ignore it if this is an outdated or invalid request
                if (ctx.selection_source != source || strcmp(mime_type, "text/plain;charset=utf-8") != 0)
                {
                    close(fd);
                    return;
                }

                char *string = (char *)env.clipboard_data.c_str();
                size_t length = env.clipboard_data.length();

                while (length > 0)
                {
                    const ssize_t result = write(fd, string, length);
                    if (result == -1)
                    {
                        if (errno == EINTR) continue;
                        LOG_ERROR("Wayland: Error while writing the clipboard: %s", strerror(errno));
                        break;
                    }

                    length -= result;
                    string += result;
                }

                close(fd);
            }

            static void data_source_handle_cancelled(void *user_data, wl_data_source *source)
            {
                wl_data_source_destroy(source);
                if (ctx.selection_source != source) return;
                ctx.selection_source = NULL;
            }

            static const struct wl_data_source_listener data_source_listener = {
                data_source_handle_target,
                data_source_handle_send,
                data_source_handle_cancelled,
            };

            static acul::string read_data_offer_as_string(wl_data_offer *offer, const char *mime_type)
            {
                int fds[2];

                if (pipe2(fds, O_CLOEXEC) == -1)
                {
                    LOG_ERROR("Wayland: Failed to create pipe for data offer: %s", strerror(errno));
                    return {};
                }

                wl_data_offer_receive(offer, mime_type, fds[1]);
                flush_display();
                close(fds[1]);

                acul::string r;
                constexpr size_t read_size = 4096;
                acul::vector<char> buffer(read_size);

                for (;;)
                {
                    ssize_t bytes_read = read(fds[0], buffer.data(), buffer.size());
                    if (bytes_read == 0) // EOF
                        break;
                    if (bytes_read < 0)
                    {
                        if (errno == EINTR) continue;

                        close(fds[0]);
                        LOG_ERROR("Wayland: Failed to read from data offer pipe: %s", strerror(errno));
                    }

                    r.append(buffer.data(), bytes_read);
                }
                close(fds[0]);
                return r;
            }

            acul::string get_clipboard_string()
            {
                if (!ctx.selection_offer)
                {
                    LOG_ERROR("Wayland: No clipboard data available");
                    return {};
                }

                if (ctx.selection_source) return env.clipboard_data;
                env.clipboard_data = read_data_offer_as_string(ctx.selection_offer, "text/plain;charset=utf-8");
                return env.clipboard_data;
            }

            void set_clipboard_string(const acul::string &text)
            {
                if (ctx.selection_source)
                {
                    wl_data_source_destroy(ctx.selection_source);
                    ctx.selection_source = NULL;
                }

                env.clipboard_data = text;

                ctx.selection_source = wl_data_device_manager_create_data_source(ctx.data_device_manager);
                if (!ctx.selection_source)
                {
                    LOG_ERROR("Wayland: Failed to create clipboard data source");
                    return;
                }
                wl_data_source_add_listener(ctx.selection_source, &data_source_listener, NULL);
                wl_data_source_offer(ctx.selection_source, "text/plain;charset=utf-8");
                wl_data_device_set_selection(ctx.data_device, ctx.selection_source, ctx.serial);
            }

            void set_window_icon(WindowData *, const acul::vector<Image> &)
            {
                LOG_WARN("Wayland: The platform does not support setting the window icon");
            }

            MonitorInfo get_primary_monitor_info()
            {
                if (ctx.outputs.empty()) return {{0, 0}, {0, 0}};
                auto &output = ctx.outputs.front();
                return {output.dimensions, output.dimensions};
            }

            void show_window(WindowData *window_data)
            {
                auto *wl_data = (WaylandWindowData *)window_data;
                if (!wl_data->libdecor_frame && !wl_data->xdg.toplevel) create_shell_objects(wl_data);
            }

            void hide_window(WindowData *window_data)
            {
                auto *wl_data = (WaylandWindowData *)window_data;
                destroy_shell_objects(wl_data);
                wl_surface_attach(wl_data->surface, NULL, 0, 0);
                wl_surface_commit(wl_data->surface);
            }

            acul::string get_window_title(WindowData *window_data) { return ((WaylandWindowData *)window_data)->title; }

            void set_window_title(WindowData *window_data, const acul::string &title)
            {
                auto *wl_data = (WaylandWindowData *)window_data;
                wl_data->title = title;
                if (wl_data->libdecor_frame)
                    libdecor_frame_set_title(wl_data->libdecor_frame, title.c_str());
                else if (wl_data->xdg.toplevel)
                    xdg_toplevel_set_title(wl_data->xdg.toplevel, title.c_str());
            }

            void enable_fullscreen(WindowData *window_data)
            {
                auto *wl_data = (WaylandWindowData *)window_data;
                if (wl_data->libdecor_frame)
                    libdecor_frame_set_fullscreen(wl_data->libdecor_frame, wl_data->output->handle);
                else if (wl_data->xdg.toplevel)
                    xdg_toplevel_set_fullscreen(wl_data->xdg.toplevel, wl_data->output->handle);
                set_idle_inhibitor(wl_data, true);
                if (wl_data->fallback.decorations) destroy_fallback_decorations(wl_data);
            }

            void disable_fullscreen(WindowData *window_data)
            {
                auto *wl_data = (WaylandWindowData *)window_data;
                if (wl_data->libdecor_frame)
                    libdecor_frame_unset_fullscreen(wl_data->libdecor_frame);
                else if (wl_data->xdg.toplevel)
                    xdg_toplevel_unset_fullscreen(wl_data->xdg.toplevel);
                set_idle_inhibitor(wl_data, false);
                if (!wl_data->libdecor_frame &&
                    wl_data->xdg.decoration_mode != ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE)
                {
                    if (wl_data->flags & WindowFlagBits::decorated) create_fallback_decorations(wl_data);
                }
            }

            acul::point2D<i32> get_cursor_position(WindowData *window_data)
            {
                return ((WaylandWindowData *)window_data)->cursor_pos;
            }

            void set_cursor_position(WindowData *window_data, acul::point2D<i32> position)
            {
                LOG_ERROR("Wayland: The platform does not support setting the cursor position");
            }

            Cursor::Platform *create_cursor(Cursor::Type type)
            {
                struct Shape
                {
                    const char *name, *fallback;
                };

                static const acul::hashmap<Cursor::Type, Shape> cursor_map = {
                    {Cursor::Type::arrow, {"default", "left_ptr"}},
                    {Cursor::Type::ibeam, {"text", "xterm"}},
                    {Cursor::Type::crosshair, {"crosshair", "crosshair"}},
                    {Cursor::Type::hand, {"pointer", "hand2"}},
                    {Cursor::Type::resize_ew, {"ew-resize", "sb_h_double_arrow"}},
                    {Cursor::Type::resize_ns, {"ns-resize", "sb_v_double_arrow"}},
                    {Cursor::Type::resize_nwse, {"nwse-resize", "bottom_right_corner"}},
                    {Cursor::Type::resize_nesw, {"nesw-resize", "bottom_left_corner"}},
                    {Cursor::Type::resize_all, {"all-scroll", "fleur"}},
                    {Cursor::Type::not_allowed, {"not-allowed", "not-allowed"}}};

                auto it = cursor_map.find(type);
                if (it == cursor_map.end()) return nullptr;
                const auto &entry = it->second;
                auto *cursor = acul::alloc<WaylandCursor>();
                cursor->handle = wl_cursor_theme_get_cursor(ctx.cursor_theme, entry.name);
                if (!cursor->handle) cursor->handle = wl_cursor_theme_get_cursor(ctx.cursor_theme, entry.fallback);
                if (!cursor->handle)
                {
                    LOG_ERROR("Wayland: Failed to create standard cursor \"%s\"", entry.name);
                    acul::release(cursor);
                    return nullptr;
                }
                return cursor;
            }

            static void assign_cursor(WaylandWindowData *window, Cursor::Platform *pd)
            {
                if (!ctx.pointer) return;
                if (window->is_cursor_hidden)
                    wl_pointer_set_cursor(ctx.pointer, ctx.pointer_enter_serial, NULL, 0, 0);
                else if (pd)
                    set_cursor_image(window, (WaylandCursor *)pd);
            }

            void assign_cursor(Window *window, Cursor::Platform *pd)
            {
                assign_cursor((WaylandWindowData *)get_window_data(*window), pd);
            }

            void destroy_cursor(Cursor::Platform *pd)
            {
                auto *wl_cursor = (WaylandCursor *)pd;
                if (wl_cursor->handle) return;
                if (wl_cursor->buffer) wl_buffer_destroy(wl_cursor->buffer);
            }

            bool is_cursor_valid(const Cursor::Platform *pd)
            {
                auto *wl_cursor = (WaylandCursor *)pd;
                return wl_cursor->handle || wl_cursor->buffer;
            }

            void hide_cursor(WindowData *window_data)
            {
                if (window_data->is_cursor_hidden) return;
                window_data->is_cursor_hidden = true;
                auto *wl_data = (WaylandWindowData *)window_data;
                assign_cursor(wl_data, get_cursor_pd(wl_data->cursor));
            }

            void show_cursor(Window *window, WindowData *window_data)
            {
                if (!window_data->is_cursor_hidden) return;
                if (window_data->cursor && window_data->cursor->valid())
                    window_data->cursor->assign(window);
                else if (platform::env.default_cursor.valid())
                    platform::env.default_cursor.assign(window);
                window_data->is_cursor_hidden = false;
            }

            acul::point2D<i32> get_window_position(WindowData *window)
            {
                LOG_ERROR("Wayland: The platform does not provide the window position");
                return {0, 0};
            }

            void set_window_position(WindowData *window, acul::point2D<i32> position)
            {
                LOG_ERROR("Wayland: The platform does not support setting the window position");
            }

            void center_window(WindowData *window)
            {
                LOG_WARN("Wayland: Cannot center window, compositor decides position");
            }

            void update_resize_limit(WindowData *window)
            {
                auto *wl_data = (WaylandWindowData *)window;
                if (ctx.libdecor.context)
                {
                    if (!wl_data->libdecor_frame) return;
                    auto resize_limit = window->resize_limit.x > 0 && window->resize_limit.y > 0
                                            ? wl_data->resize_limit
                                            : acul::point2D<i32>{0, 0};
                    libdecor_frame_set_min_content_size(wl_data->libdecor_frame, resize_limit.x, resize_limit.y);
                    auto size = window->dimenstions;
                    libdecor_state *state = libdecor_state_new(size.x, size.y);
                    libdecor_frame_commit(wl_data->libdecor_frame, state, NULL);
                    libdecor_state_free(state);
                }
                else if (wl_data->xdg.toplevel)
                    update_xdg_size_limits(wl_data);
            }

            void minimize_window(WindowData *window)
            {
                auto *wl_data = (WaylandWindowData *)window;
                if (wl_data->libdecor_frame)
                    libdecor_frame_set_minimized(wl_data->libdecor_frame);
                else if (wl_data->xdg.toplevel)
                    xdg_toplevel_set_minimized(wl_data->xdg.toplevel);
            }

            void maximize_window(WindowData *window)
            {
                auto *wl_data = (WaylandWindowData *)window;
                if (wl_data->flags & WindowFlagBits::minimized)
                    return; // There is no way to unset minimized, so there is nothing to do in this case.
                if (wl_data->flags & WindowFlagBits::maximized)
                {
                    if (wl_data->libdecor_frame)
                        libdecor_frame_unset_maximized(wl_data->libdecor_frame);
                    else if (wl_data->xdg.toplevel)
                        xdg_toplevel_unset_maximized(wl_data->xdg.toplevel);
                    else
                        window->flags &= ~WindowFlagBits::maximized;
                }
                else
                {
                    if (wl_data->libdecor_frame)
                        libdecor_frame_set_maximized(wl_data->libdecor_frame);
                    else if (wl_data->xdg.toplevel)
                        xdg_toplevel_set_maximized(wl_data->xdg.toplevel);
                }
            }
        } // namespace wayland
    } // namespace platform

    namespace native_access
    {
        wl_surface *get_wayland_surface(const Window &window)
        {
            auto *wl_data = (platform::wayland::WaylandWindowData *)get_window_data(window);
            return wl_data->surface;
        }

#ifdef AWIN_TEST_BUILD
        void enable_wayland_surface_placeholder() { platform::wayland::ctx.is_surface_placeholder_enabled = true; }
#endif
    } // namespace native_access
} // namespace awin