#pragma once
#include <acul/string/string.hpp>
#include <awin/window.hpp>
#include "awin/types.hpp"

namespace awin
{
    namespace platform
    {
        extern APPLIB_API struct WindowEnvironment
        {
            acul::string clipboard_data; // Clipboard data storage.
            struct Timer
            {
#ifndef _WIN32
                clockid_t clock_id;
#endif
                u64 offset;                   // Time offset
                u64 frequency;                // Timer frequency
            } timer;                          // Timer information for time tracking.
            f64 timeout = WINDOW_TIMEOUT_INF; // Global timeout for waking up the main loop.
            acul::events::dispatcher *ed = nullptr;
            Cursor default_cursor;
        } env;

        // A structure that contains event-related data and listeners for window events
        extern struct EventRegistry
        {
#ifdef _WIN32
            acul::events::event_group *ncl_mouse_down;
            acul::events::event_group *nc_hit_test;
#endif
            acul::events::event_group *focus;
            acul::events::event_group *char_input;
            acul::events::event_group *key_input;
            acul::events::event_group *mouse_click;
            acul::events::event_group *mouse_enter;
            acul::events::event_group *mouse_move_delta;
            acul::events::event_group *mouse_move;
            acul::events::event_group *scroll;
            acul::events::event_group *minimize;
            acul::events::event_group *maximize;
            acul::events::event_group *resize;
            acul::events::event_group *move;
            acul::events::event_group *dpi_changed;
        } event_registry;
    } // namespace platform

    inline WindowData *get_window_data(const Window &window) { return window._data; }
} // namespace awin