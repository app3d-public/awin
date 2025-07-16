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
            // Handling non-client area mouse clicks on Windows.
            acul::events::listener<Win32NativeEvent> *ncl_mouse_down;

            // Performing hit testing in the non-client area on Windows.
            acul::events::listener<Win32NativeEvent> *nc_hit_test;
#endif
            // List of event listeners for focus-related events.
            acul::vector<acul::events::listener<FocusEvent> *> focus;

            // List of event listeners for character input events.
            acul::vector<acul::events::listener<CharInputEvent> *> char_input;

            // List of event listeners for keyboard input events.
            acul::vector<acul::events::listener<KeyInputEvent> *> key_input;

            // List of event listeners for mouse click events.
            acul::vector<acul::events::listener<MouseClickEvent> *> mouse_click;

            // List of event listeners for cursor enter/leave events.
            acul::vector<acul::events::listener<MouseEnterEvent> *> mouse_enter;

            //  Listener for handling when the mouse position changes in RAW Input mode.
            acul::vector<acul::events::listener<PosEvent> *> mouse_move_delta;

            //  Listener for handling when the mouse position changes in absolute (per Window dimensions) values
            acul::vector<acul::events::listener<PosEvent> *> mouse_move;

            // List of event listeners for scroll events.
            acul::vector<acul::events::listener<ScrollEvent> *> scroll;

            // List of event listeners for window minimize events.
            acul::vector<acul::events::listener<StateEvent> *> minimize;

            // List of event listeners for window maximize events.
            acul::vector<acul::events::listener<StateEvent> *> maximize;

            // List of event listeners for window resize events.
            acul::vector<acul::events::listener<PosEvent> *> resize;

            // List of event listeners for window move events.
            acul::vector<acul::events::listener<PosEvent> *> move;

            // List of event listeners for DPI (dots per inch) changed events.
            acul::vector<acul::events::listener<DpiChangedEvent> *> dpi_changed;

        } event_registry;

        /**
         * dispatchs a window event to the specified listeners.
         *
         * @param listener a list of event listeners
         * @param args event arguments
         */
        template <typename T, typename... Args>
        inline void dispatch_window_event(const acul::vector<acul::events::listener<T> *> &listener, Args &&...args)
        {
            T event(std::forward<Args>(args)...);
            for (const auto &l : listener) l->invoke(event);
        }
    } // namespace platform

    inline WindowData *get_window_data(const Window &window) { return window._data; }
} // namespace awin