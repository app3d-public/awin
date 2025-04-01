#pragma once

#include <acul/event.hpp>
#include <acul/point2d.hpp>
#include "types.hpp"
#ifdef _WIN32
    #include "win32/platform.hpp"
#endif

#define WINDOW_TIMEOUT_INF -1

namespace awin
{
    class Window;
    namespace platform
    {
        // The window environment configuration for window management and interactions within the windowing system.
        extern APPLIB_API struct WindowEnvironment
        {
            acul::string clipboardData; // Clipboard data storage.
            struct Timer
            {
                u64 offset;                   // Time offset
                u64 frequency;                // Timer frequency
            } timer;                          // Timer information for time tracking.
            f64 timeout = WINDOW_TIMEOUT_INF; // Global timeout for waking up the main loop.
            acul::events::dispatcher *ed = nullptr;
        } env;

        struct WindowData
        {
            Window *owner;
            acul::point2D<i32> dimenstions;
            CreationFlags flags;
            bool isCursorHidden{false};
            bool focused{false};
            bool readyToClose = false;
            acul::point2D<i32> resizeLimit{0, 0};
            io::KeyPressState keys[io::Key::kLast + 1];
            Cursor *cursor;
            platform_data_t backend;
        };
    } // namespace platform

    inline void updateTimeout(f64 timeout) { platform::env.timeout = std::max(platform::env.timeout, timeout); }
} // namespace awin