/***********************************************************************************
 * App3D Window Library
 * A library for creating and managing 3D windows within the App3D environment.
 * ----------------------------------------------------------------------------------
 * Copyright (c) 2023, Wusiki Jeronii <wusikijeronii@gmail.com>
 ***********************************************************************************/

#ifndef APP_WINDOW_WINDOW_H
#define APP_WINDOW_WINDOW_H

#include <core/event/event.hpp>
#include <core/std/basic_types.hpp>
#include <string>
#include "types.hpp"
#ifdef _WIN32
    #include <windows.h>
#endif

#define WINDOW_DONT_CARE -1

namespace window
{
    // Data structures and platform-specific functions that are utilized by internal Window API and offer secure data
    // exchange with external API interfaces.
    namespace platform
    {
        //  Data and state related to a specific window instance. It acts as a direct representative of a window in the
        //  platform's context.
        struct WindowPlatformData;

        // Captures the necessary context for platform initialization and overall operation, storing global settings and
        // state that are essential for the platform's functionality within the windowing system.
        struct PlatformContext;

        // Serves as a gateway for higher-level APIs to interact with platform-specific window data, abstracting the
        // details of the platform to provide a consistent interface for window manipulation.
        class AccessBridge;

        // The window environment configuration for window management and interactions within the windowing system.
        extern struct WindowEnvironment
        {
            PlatformContext *context;  // Platform-specific context for window operations.
            std::string clipboardData; // Clipboard data storage.
            struct Timer
            {
                u64 offset;    // Time offset
                u64 frequency; // Timer frequency
            } timer;           // Timer information for time tracking.
        } env;

        // Initializes the platform-specific components and sets up the windowing system environment.
        bool initPlatform();

        // Initializes the timer subsystem to enable precise time management for various window-related operations.
        void initTimer();

        // Retrieves the current time value from the timer subsystem, which is crucial for time-dependent calculations.
        u64 getTimeValue();

        // Retrieves the timer frequency, an essential parameter for accurate time-based calculations.
        u64 getTimeFrequency();

        // Performs cleanup and releases platform-specific resources associated with the windowing system environment.
        void destroyPlatform();

        // A utility function responsible for processing keyboard input events specific to a particular window
        // implementation. It manages key presses, releases, and key modifiers, facilitating their propagation to the
        // appropriate event handlers.
        void inputKey(WindowPlatformData *impl, io::Key key, io::KeyPressState action, io::KeyMode mods);
    } // namespace platform

    // A window entity in the windowing system
    class Window
    {
    public:
        // Initialize a window with a title, dimensions, and creation flags.
        explicit Window(const std::string &title, i32 width = WINDOW_DONT_CARE, i32 height = WINDOW_DONT_CARE,
               CreationFlags flags = WINDOW_DEFAULT_FLAGS);

        ~Window();

        // Get the window title
        std::string title() const;

        // Set the window title
        void title(const std::string &title);

        // Returns the width of the window.
        Point2D dimensions() const;

        // Check if the window has decorations
        bool decorated() const;

        // Check if the window is resizable.
        bool resizable() const;

        // Check if the window is in fullscreen mode.
        bool fullscreen() const;

        // Enable fullscreen mode.
        void enableFullscreen();

        // Disable fullscreen mode.
        void disableFullscreen();

        // Get the current cursor position.
        Point2D cursorPosition() const;

        // Set the cursor position
        void cursorPosition(Point2D position);

        // Show the cursor.
        void showCursor();

        // Hide the cursor.
        void hideCursor();

        // Set cursor
        void setCursor(Cursor *cursor);

        // Check if the window is focused.
        bool focused() const;

        // Check if the window is minimized.
        bool minimized() const;

        // Check if the window is maximized.
        bool maximized() const;

        // Check if the window is hidden.
        bool hidden() const;

        // Get the window's resize limits.
        Point2D resizeLimit() const;

        // Set the window's resize limits.
        void resizeLimit(i32 width, i32 height);

        // Check if the window is ready to be closed.
        bool readyToClose() const;

        // Show the window if it is hidden.
        void showWindow();

        // Hide the window
        void hideWindow();

        // Get current window position
        Point2D windowPos() const;

        // Set window position
        void windowPos(Point2D position);

        // Center the window to the parent
        void centerWindowPos();

        // Retrieves the access bridge for interfacing with platform-specific window data.
        const platform::AccessBridge &accessBridge() const { return *_accessBridge; }

        friend Point2D getWindowSize(const Window &window);
        friend std::string getClipboardString(const Window &window);
        friend void setClipboardString(const Window &window, const std::string &text);

    private:
        platform::WindowPlatformData *_platform;
        const platform::AccessBridge *_accessBridge;
    };

    // Updates the event registry by associating different types of window events with their listeners.
    // This function gathers listeners for various event types like mouse clicks, keyboard input,
    // focus changes, etc., and registers them to corresponding events. This allows the system
    // to efficiently dispatch events to the appropriate listeners as they occur.
    // Specific actions are taken for different platforms (e.g., Windows-specific event listeners)
    // to ensure compatibility and proper handling across different operating systems.
    void updateEvents();

    // Events
#ifdef _WIN32
    // Event specific to Win32 platform, carrying native window message data.
    struct Win32NativeEvent : public Event
    {
        window::Window *window; // Pointer to the associated Window object.
        UINT uMsg;              // Windows message identifier.
        WPARAM wParam;          // Additional message information.
        LPARAM lParam;          // Additional message information.
        LRESULT *lResult;       // Pointer to the result of the message processing.

        explicit Win32NativeEvent(const std::string &name = "", window::Window *window = nullptr, UINT uMsg = 0,
                                  WPARAM wParam = 0, LPARAM lParam = 0, LRESULT *lResult = nullptr)
            : Event(name), window(window), uMsg(uMsg), wParam(wParam), lParam(lParam), lResult(lResult)
        {
        }
    };
#endif

    // Represents a focus change event in a window.
    struct FocusEvent : public Event
    {
        // Pointer to the associated Window object.
        window::Window *window;

        // Whether the window is focused or not.
        bool focused;

        explicit FocusEvent(const std::string &name = "", window::Window *window = nullptr, bool focused = false)
            : Event(name), window(window), focused(focused)
        {
        }
    };

    // Represents a character input event in a window.
    struct CharInputEvent : public Event
    {
        window::Window *window; // Pointer to the associated Window object.
        u32 charCode;           // Unicode character code.

        explicit CharInputEvent(const std::string &name = "", window::Window *window = nullptr, u32 charCode = 0)
            : Event(name), window(window), charCode(charCode)
        {
        }
    };

    // Represents a keyboard input event in a window.
    struct KeyInputEvent : public Event
    {
        window::Window *window;   // Pointer to the associated Window object.
        io::Key key;              // The key involved in the event.
        io::KeyPressState action; // The action (press, release, repeat) associated with the key.
        io::KeyMode mods;         // The key modifiers associated with the key.

        explicit KeyInputEvent(const std::string &name = "", window::Window *window = nullptr,
                               io::Key key = io::Key::kUnknown, io::KeyPressState action = io::KeyPressState::release,
                               io::KeyMode mods = io::KeyMode{})
            : Event(name), window(window), key(key), action(action), mods(mods)
        {
        }
    };

    // Represents a mouse click event in a window.
    struct MouseClickEvent : public Event
    {
        window::Window *window;   // Pointer to the associated Window object.
        io::MouseKey button;      // The mouse button involved in the event.
        io::KeyPressState action; // The action (press, release, repeat) associated with the key.
        io::KeyMode mods;         // The key modifiers associated with the key.

        explicit MouseClickEvent(const std::string &name = "", window::Window *window = nullptr,
                                 io::MouseKey button = io::MouseKey::unknown,
                                 io::KeyPressState action = io::KeyPressState::release,
                                 io::KeyMode mods = io::KeyMode{})
            : Event(name), window(window), button(button), action(action), mods(mods)
        {
        }
    };

    // Represents a mouse position event in a window. This one is emitted when the mouse enters or leaves the window.
    struct CursorEnterEvent : public Event
    {
        window::Window *window; // Pointer to the associated Window object.
        bool entered;           // Whether the mouse entered or left the window.

        explicit CursorEnterEvent(const std::string &name = "", window::Window *window = nullptr, bool entered = false)
            : Event(name), window(window), entered(entered)
        {
        }
    };

    // Represents a position change event in a window.
    struct PosEvent : public Event
    {
        window::Window *window; // Pointer to the associated Window object.
        Point2D position;       // The new position.

        explicit PosEvent(const std::string &name = "", window::Window *window = nullptr, Point2D position = Point2D())
            : Event(name), window(window), position(position)
        {
        }
    };

    // Represents a scroll event in a window.
    struct ScrollEvent : public Event
    {
        window::Window *window; // Pointer to the associated Window object.
        f32 offset;             // The scroll offset.

        explicit ScrollEvent(const std::string &name = "", window::Window *window = nullptr, f32 offset = 0.0f)
            : Event(name), window(window), offset(offset)
        {
        }
    };

    // Represents a DPI change event in a window.
    struct DpiChangedEvent : public Event
    {
        window::Window *window; // Pointer to the associated Window object.
        f32 xDpi;               // New DPI value in the x-axis.
        f32 yDpi;               // New DPI value in the y-axis.

        explicit DpiChangedEvent(const std::string &name = "", window::Window *window = nullptr, f32 xDpi = 0.0f,
                                 f32 yDpi = 0.0f)
            : Event(name), window(window), xDpi(xDpi), yDpi(yDpi)
        {
        }
    };

    // A structure that contains event-related data and listeners for window events
    extern struct DefaultRegistry
    {
        // App3D event manager
        EventManager *mng;
#ifdef _WIN32
        // Handling non-client area mouse clicks on Windows.
        EventListener<Win32NativeEvent> *NCLMouseClick;

        // Performing hit testing in the non-client area on Windows.
        EventListener<Win32NativeEvent> *NCHitTest;
#endif
        // List of event listeners for focus-related events.
        Array<std::shared_ptr<EventListener<FocusEvent>>> focusEvents;

        // List of event listeners for character input events.
        Array<std::shared_ptr<EventListener<CharInputEvent>>> charInputEvents;

        // List of event listeners for keyboard input events.
        Array<std::shared_ptr<EventListener<KeyInputEvent>>> keyInputEvents;

        // List of event listeners for mouse click events.
        Array<std::shared_ptr<EventListener<MouseClickEvent>>> mouseClickEvents;

        // List of event listeners for cursor enter/leave events.
        Array<std::shared_ptr<EventListener<CursorEnterEvent>>> cursorEnterEvents;

        //  Listener for handling when the mouse position changes in RAW Input mode.
        Array<std::shared_ptr<EventListener<PosEvent>>> cursorPosEvents;

        //  Listener for handling when the mouse position changes in absolute (per Window dimensions) values
        Array<std::shared_ptr<EventListener<PosEvent>>> cursorPosAbsEvents;

        // List of event listeners for scroll events.
        Array<std::shared_ptr<EventListener<ScrollEvent>>> scrollEvents;

        // List of event listeners for window minimize events.
        Array<std::shared_ptr<EventListener<PosEvent>>> minimizeEvents;

        // List of event listeners for window maximize events.
        Array<std::shared_ptr<EventListener<PosEvent>>> maximizeEvents;

        // List of event listeners for window resize events.
        Array<std::shared_ptr<EventListener<PosEvent>>> resizeEvents;

        // List of event listeners for window move events.
        Array<std::shared_ptr<EventListener<PosEvent>>> moveEvents;

        // List of event listeners for DPI (dots per inch) changed events.
        Array<std::shared_ptr<EventListener<DpiChangedEvent>>> dpiChangedEvents;

    } eventRegistry;

    /**
     * Emits a window event to the specified listeners.
     *
     * @param listener a list of event listeners
     * @param args event arguments
     */
    template <typename T, typename... Args>
    inline void emitWindowEvent(const Array<std::shared_ptr<EventListener<T>>> &listener, Args &&...args)
    {
        T event(std::forward<Args>(args)...);
        for (const auto &l : listener)
            l->invoke(event);
    }

    // Get the time elapsed since library initialization in seconds as a floating-point value.
    f64 getTime();

    // Set the window library initialization time to the specified value in seconds.
    void setTime(f64 time);

    /**
     * Retrieves information about the primary monitor of the system. The primary monitor
     * is typically the default display where applications are opened by default. This function
     * is particularly useful for applications that need to be aware of the monitor's
     * resolution, position, and size for layout optimizations, window positioning, or
     * adapting to different screen sizes.
     *
     * The function returns a MonitorInfo structure, which contains the following fields:
     * - xpos: The X-coordinate of the top-left corner of the primary monitor relative to
     *         the virtual screen (which may span multiple monitors).
     * - ypos: The Y-coordinate of the top-left corner of the primary monitor.
     * - width: The width of the primary monitor's visible display area in pixels.
     * - height: The height of the primary monitor's visible display area in pixels.
     *
     * These details are crucial for handling screen coordinates and dimensions effectively,
     * especially in multi-monitor setups. The function can be used to determine the
     * optimal size and position for application windows, ensuring compatibility across
     * different monitor configurations.
     *
     * @return MonitorInfo - A structure containing the position and size of the primary monitor.
     */
    MonitorInfo getPrimaryMonitorInfo();

    // Processes all pending events in the event queue. This function checks the state
    // of all windows and other event sources, processes those events, and returns
    // control after all events have been processed. Typically used in an application's
    // update loop to handle events as they occur.
    void pollEvents();

    // Waits for new events to occur and processes them as soon as they appear.
    // Unlike pollEvents, this function blocks the execution of the program until
    // new events are available. Useful in situations where you want to conserve CPU
    // usage when the application is idle or when you need to wait for user input
    // without continuously polling.
    void waitEvents();

    // Waits for events with a specified timeout and processes them. If no events occur
    // within the given timeout period, the function returns. This is useful for
    // scenarios where you want to wait for events but also perform some other action
    // if no events occur within a certain time frame, such as updating the UI or
    // handling non-event-related logic.
    void waitEventsTimeout(f64 timeout);

    // Retrieves the current dots per inch (DPI) value of the display.
    f32 getDpi();

    // Get the client area size
    Point2D getWindowSize(const Window &window);

    // Get text string from the clipboard buffer
    std::string getClipboardString(const Window &window);

    // Set text string in the clipboard buffer
    void setClipboardString(const Window &window, const std::string &text);

    // Initialize the library with the provided event manager.
    void initLibrary(EventManager &events);

    // Destroy the library and release associated resources.
    void destroyLibrary();

    namespace platform
    {
        struct WindowPlatformBase
        {
            Window *owner;
            Point2D dimenstions;
            CreationFlags flags;
            bool isCursorHidden{false};
            bool focused{false};
            bool minimized{false};
            bool maximized{false};
            bool readyToClose = false;
            Point2D resizeLimit{0, 0};
            io::KeyPressState keys[io::Key::kLast + 1];
            Cursor *cursor;

            virtual ~WindowPlatformBase() = default;
        };
    } // namespace platform
} // namespace window
#endif