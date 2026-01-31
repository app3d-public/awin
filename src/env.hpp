#pragma once
#include <acul/string/string.hpp>
#include <awin/window.hpp>
#include "awin/types.hpp"

namespace awin
{
    namespace platform
    {
        // A structure that contains event-related data and listeners for window events
        struct EventRegistry
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
        };

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
            acul::log::log_service *log_service = nullptr;
            acul::log::logger_base *logger = nullptr;
            Cursor default_cursor;
            EventRegistry events;
        } *g_env;
    } // namespace platform

    inline WindowData *get_window_data(const Window &window) { return window._data; }
} // namespace awin

#define AWIN_LOG_DEFAULT(level, ...) \
    acul::log::write(platform::g_env->log_service, platform::g_env->logger, level, __VA_ARGS__)
#define AWIN_LOG_INFO(...)  AWIN_LOG_DEFAULT(acul::log::level::info, __VA_ARGS__)
#define AWIN_LOG_DEBUG(...) AWIN_LOG_DEFAULT(acul::log::level::debug, __VA_ARGS__)
#define AWIN_LOG_TRACE(...) AWIN_LOG_DEFAULT(acul::log::level::trace, __VA_ARGS__)
#define AWIN_LOG_WARN(...)  AWIN_LOG_DEFAULT(acul::log::level::warn, __VA_ARGS__)
#define AWIN_LOG_ERROR(...) AWIN_LOG_DEFAULT(acul::log::level::error, __VA_ARGS__)
#define AWIN_LOG_FATAL(...) AWIN_LOG_DEFAULT(acul::log::level::fatal, __VA_ARGS__)