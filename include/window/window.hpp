#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <core/event/event.hpp>
#include <core/log.hpp>
#include "types.hpp"
#ifdef _WIN32
    #include "window_win32.hpp"
#else
    #error "Unsupported platform"
#endif

namespace window
{
#ifdef _WIN32
    using Window = Win32Window;
#else
    #error "Unsupported platform"
#endif

    inline void initLibrary(EventManager &events)
    {
        if (!_internal::initPlatform())
            throw std::runtime_error("Failed to initialize Window platform");
        _internal::gWindowEvents.e = &events;
    }

    inline void destroyLibrary()
    {
        logInfo("Destroying Window platform");
        _internal::destroyPlatform();
        _internal::gWindowEvents.e = nullptr;
    }

    // Updates the event registry by associating different types of window events with their listeners.
    // This function gathers listeners for various event types like mouse clicks, keyboard input,
    // focus changes, etc., and registers them to corresponding events. This allows the system
    // to efficiently dispatch events to the appropriate listeners as they occur.
    // Specific actions are taken for different platforms (e.g., Windows-specific event listeners)
    // to ensure compatibility and proper handling across different operating systems.
    void updateEvents();
} // namespace window

#endif