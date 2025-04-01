/***********************************************************************************
 * App3D Window Library
 * A library for creating and managing 3D windows within the App3D environment.
 * ----------------------------------------------------------------------------------
 * Copyright (c) 2023, Wusiki Jeronii <wusikijeronii@gmail.com>
 ***********************************************************************************/

#ifndef APP_WINDOW_WINDOW_H
#define APP_WINDOW_WINDOW_H

#include <acul/event.hpp>
#include "types.hpp"
#ifdef _WIN32
    #include "platform.hpp"
#endif

#define WINDOW_DONT_CARE -1

namespace awin
{
    // Data structures and platform-specific functions that are utilized by internal Window API and offer secure data
    // exchange with external API interfaces.
    namespace platform
    {
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
        void inputKey(WindowData *data, io::Key key, io::KeyPressState action, io::KeyMode mods);
    } // namespace platform

    // A window entity in the windowing system
    class APPLIB_API Window
    {
    public:
        // Initialize a window with a title, dimensions, and creation flags.
        explicit Window(const acul::string &title, i32 width = WINDOW_DONT_CARE, i32 height = WINDOW_DONT_CARE,
                        CreationFlags flags = WINDOW_DEFAULT_FLAGS);

        // Destroy the window
        void destroy();

        // Get the window title
        acul::string title() const;

        // Set the window title
        void title(const acul::string &title);

        // Returns the width of the window.
        acul::point2D<i32> dimensions() const { return _platform.dimenstions; }

        // Check if the window has decorations
        inline bool decorated() const { return (_platform.flags & awin::CreationFlagsBits::decorated) != 0; }

        // Check if the window is resizable.
        inline bool resizable() const { return (_platform.flags & awin::CreationFlagsBits::resizable) != 0; }

        // Check if the window is in fullscreen mode.
        bool fullscreen() const { return (_platform.flags & awin::CreationFlagsBits::fullscreen) != 0; }

        // Enable fullscreen mode.
        void enableFullscreen();

        // Disable fullscreen mode.
        void disableFullscreen();

        // Get the current cursor position.
        acul::point2D<i32> cursorPosition() const;

        // Set the cursor position
        void cursorPosition(acul::point2D<i32> position);

        // Show the cursor.
        void showCursor();

        // Hide the cursor.
        void hideCursor();

        // Check if the cursor is hidden.
        inline bool isCursorHidden() const { return _platform.isCursorHidden; }

        // Set cursor
        inline void setCursor(Cursor *cursor) { _platform.cursor = cursor; }

        // Check if the window is focused.
        inline bool focused() const { return _platform.focused; }

        // Check if the window is minimized.
        inline bool minimized() const { return _platform.flags & awin::CreationFlagsBits::minimized; }

        // Minimize the window
        void minimize();

        // Check if the window is maximized.
        inline bool maximized() const { return _platform.flags & awin::CreationFlagsBits::maximized; }

        // Maximize the window
        void maximize();

        // Check if the window is hidden.
        inline bool hidden() const { return (_platform.flags & awin::CreationFlagsBits::hidden) != 0; }

        // Get the window's resize limits.
        inline acul::point2D<i32> resizeLimit() const { return _platform.resizeLimit; }

        // Set the window's resize limits.
        inline void resizeLimit(i32 width, i32 height) { _platform.resizeLimit = {width, height}; }

        // Check if the window is ready to be closed.
        inline bool readyToClose() const { return _platform.readyToClose; }

        // Change the window's ready-to-close state.
        inline void readyToClose(bool readyToClose) { _platform.readyToClose = readyToClose; }

        // Show the window if it is hidden.
        void showWindow();

        // Hide the window
        void hideWindow();

        // Get current window position
        acul::point2D<i32> windowPos() const;

        // Set window position
        void windowPos(acul::point2D<i32> position);

        // Center the window to the parent
        void centerWindowPos();

    private:
        platform::WindowData _platform;

        friend platform::native_access;
    };

    // Updates the event registry by associating different types of window events with their listeners.
    // This function gathers listeners for various event types like mouse clicks, keyboard input,
    // focus changes, etc., and registers them to corresponding events. This allows the system
    // to efficiently dispatch events to the appropriate listeners as they occur.
    // Specific actions are taken for different platforms (e.g., Windows-specific event listeners)
    // to ensure compatibility and proper handling across different operating systems.
    APPLIB_API void updateEvents();

    // Events
    namespace event_id
    {
        enum : u64
        {
            none = 0x0,
#ifdef _WIN32
            NCHitTest = 0x2D5AA1F9EE962269,
            NCMouseDown = 0x12D7ACB8440B7678,
#endif
            focus = 0x05AC2ABF9E301AD1,
            charInput = 0x0B37F6873EA5B017,
            keyInput = 0x0E8A91707EFCEB90,
            mouseClick = 0x06254FC551B67986,
            mouseEnter = 0x1DFE0E9A4D85B1EE,
            mouseMove = 0x15F068FC45DB86CE,
            mouseMoveAbs = 0x037E8253212E7276,
            scroll = 0x0D66A892FC053357,
            dpiChanged = 0x37516DB961C1BF7A,
            minimize = 0x16AB16E6670A5AC2,
            maximize = 0x0A8C9013D84CEC08,
            resize = 0x1FB82ED0F4C701CB,
            move = 0x2A5416AB994F5AAE
        };
    }; // namespace event_id

#ifdef _WIN32
    // Event specific to Win32 platform, carrying native window message data.
    struct Win32NativeEvent : acul::events::event
    {
        awin::Window *window; // Pointer to the associated Window object.
        HWND hwnd;            // Handle to the window.
        UINT uMsg;            // Windows message identifier.
        WPARAM wParam;        // Additional message information.
        LPARAM lParam;        // Additional message information.
        LRESULT lResult;      // Pointer to the result of the message processing.

        explicit Win32NativeEvent(u64 id = 0, awin::Window *window = nullptr, HWND hwnd = 0, UINT uMsg = 0,
                                  WPARAM wParam = 0, LPARAM lParam = 0, LRESULT lResult = -1)
            : event(id), window(window), hwnd(hwnd), uMsg(uMsg), wParam(wParam), lParam(lParam), lResult(lResult)
        {
        }
    };
#endif

    // Represents a focus change event in a window.
    struct FocusEvent : public acul::events::event
    {
        // Pointer to the associated Window object.
        awin::Window *window;

        // Whether the window is focused or not.
        bool focused;

        explicit FocusEvent(awin::Window *window = nullptr, bool focused = false)
            : event(event_id::focus), window(window), focused(focused)
        {
        }
    };

    // Represents a character input event in a window.
    struct CharInputEvent : public acul::events::event
    {
        awin::Window *window; // Pointer to the associated Window object.
        u32 charCode;         // Unicode character code.

        explicit CharInputEvent(awin::Window *window = nullptr, u32 charCode = 0)
            : event(event_id::charInput), window(window), charCode(charCode)
        {
        }
    };

    // Represents a keyboard input event in a window.
    struct KeyInputEvent : acul::events::event
    {
        awin::Window *window;     // Pointer to the associated Window object.
        io::Key key;              // The key involved in the event.
        io::KeyPressState action; // The action (press, release, repeat) associated with the key.
        io::KeyMode mods;         // The key modifiers associated with the key.

        explicit KeyInputEvent(awin::Window *window = nullptr, io::Key key = io::Key::kUnknown,
                               io::KeyPressState action = io::KeyPressState::release, io::KeyMode mods = io::KeyMode{})
            : event(event_id::keyInput), window(window), key(key), action(action), mods(mods)
        {
        }
    };

    // Represents a mouse click event in a window.
    struct MouseClickEvent : public acul::events::event
    {
        awin::Window *window;     // Pointer to the associated Window object.
        io::MouseKey button;      // The mouse button involved in the event.
        io::KeyPressState action; // The action (press, release, repeat) associated with the key.
        io::KeyMode mods;         // The key modifiers associated with the key.

        explicit MouseClickEvent(awin::Window *window = nullptr, io::MouseKey button = io::MouseKey::unknown,
                                 io::KeyPressState action = io::KeyPressState::release,
                                 io::KeyMode mods = io::KeyMode{})
            : event(event_id::mouseClick), window(window), button(button), action(action), mods(mods)
        {
        }
    };

    // Represents a mouse position event in a window. This one is dispatchted when the mouse enters or leaves the
    // window.
    struct MouseEnterEvent : public acul::events::event
    {
        awin::Window *window; // Pointer to the associated Window object.
        bool entered;         // Whether the mouse entered or left the window.

        explicit MouseEnterEvent(awin::Window *window = nullptr, bool entered = false)
            : event(event_id::mouseEnter), window(window), entered(entered)
        {
        }
    };

    // Represents a window state change event in a window.
    struct StateEvent : public acul::events::event
    {
        awin::Window *window; // Pointer to the associated Window object.
        bool state;           // The new state.

        explicit StateEvent(u64 id, awin::Window *window = nullptr, bool state = false)
            : event(id), window(window), state(state)
        {
        }
    };

    // Represents a position change event in a window.
    struct PosEvent : public acul::events::event
    {
        awin::Window *window;        // Pointer to the associated Window object.
        acul::point2D<i32> position; // The new position.

        explicit PosEvent(u64 id = 0, awin::Window *window = nullptr, acul::point2D<i32> position = {})
            : event(id), window(window), position(position)
        {
        }
    };

    // Represents a scroll event in a window.
    struct ScrollEvent : public acul::events::event
    {
        awin::Window *window; // Pointer to the associated Window object.
        f32 h;                // The Horizontal scroll value.
        f32 v;                // The Vertical scroll value.

        explicit ScrollEvent(awin::Window *window = nullptr, f32 hscroll = 0.0f, f32 vscroll = 0.0f)
            : event(event_id::scroll), window(window), h(hscroll), v(vscroll)
        {
        }
    };

    // Represents a DPI change event in a window.
    struct DpiChangedEvent : public acul::events::event
    {
        awin::Window *window; // Pointer to the associated Window object.
        f32 xDpi;             // New DPI value in the x-axis.
        f32 yDpi;             // New DPI value in the y-axis.

        explicit DpiChangedEvent(awin::Window *window = nullptr, f32 xDpi = 0.0f, f32 yDpi = 0.0f)
            : event(event_id::dpiChanged), window(window), xDpi(xDpi), yDpi(yDpi)
        {
        }
    };

    // A structure that contains event-related data and listeners for window events
    extern struct DefaultRegistry
    {
#ifdef _WIN32
        // Handling non-client area mouse clicks on Windows.
        acul::events::listener<Win32NativeEvent> *NCLMouseDown;

        // Performing hit testing in the non-client area on Windows.
        acul::events::listener<Win32NativeEvent> *NCHitTest;
#endif
        // List of event listeners for focus-related events.
        acul::vector<acul::events::listener<FocusEvent> *> focus;

        // List of event listeners for character input events.
        acul::vector<acul::events::listener<CharInputEvent> *> charInput;

        // List of event listeners for keyboard input events.
        acul::vector<acul::events::listener<KeyInputEvent> *> keyInput;

        // List of event listeners for mouse click events.
        acul::vector<acul::events::listener<MouseClickEvent> *> mouseClick;

        // List of event listeners for cursor enter/leave events.
        acul::vector<acul::events::listener<MouseEnterEvent> *> mouseEnter;

        //  Listener for handling when the mouse position changes in RAW Input mode.
        acul::vector<acul::events::listener<PosEvent> *> mouseMove;

        //  Listener for handling when the mouse position changes in absolute (per Window dimensions) values
        acul::vector<acul::events::listener<PosEvent> *> mouseMoveAbs;

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
        acul::vector<acul::events::listener<DpiChangedEvent> *> dpiChanged;

    } eventRegistry;

    /**
     * dispatchs a window event to the specified listeners.
     *
     * @param listener a list of event listeners
     * @param args event arguments
     */
    template <typename T, typename... Args>
    inline void dispatchWindowEvent(const acul::vector<acul::events::listener<T> *> &listener, Args &&...args)
    {
        T event(std::forward<Args>(args)...);
        for (const auto &l : listener) l->invoke(event);
    }

    // Get the time elapsed since library initialization in seconds as a floating-point value.
    APPLIB_API f64 getTime();

    // Set the window library initialization time to the specified value in seconds.
    APPLIB_API void setTime(f64 time);

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
    APPLIB_API MonitorInfo getPrimaryMonitorInfo();

    // Processes all pending events in the event queue. This function checks the state
    // of all windows and other event sources, processes those events, and returns
    // control after all events have been processed. Typically used in an application's
    // update loop to handle events as they occur.
    APPLIB_API void pollEvents();

    // Waits for new events to occur and processes them as soon as they appear.
    // Unlike pollEvents, this function blocks the execution of the program until
    // new events are available. Useful in situations where you want to conserve CPU
    // usage when the application is idle or when you need to wait for user input
    // without continuously polling.
    APPLIB_API void waitEvents();

    // Waits for events with a global timeout and processes them. If no events occur
    // within the given timeout period, the function returns. This is useful for
    // scenarios where you want to wait for events but also perform some other action
    // if no events occur within a certain time frame, such as updating the UI or
    // handling non-event-related logic.
    APPLIB_API void waitEventsTimeout();

    // Pushes an empty event to the event queue.
    APPLIB_API void pushEmptyEvent();

    // Retrieves the current dots per inch (DPI) value of the display.
    APPLIB_API f32 getDpi();

    // Get the client area size
    APPLIB_API acul::point2D<i32> getWindowSize(const Window &window);

    // Get text string from the clipboard buffer
    APPLIB_API acul::string getClipboardString(const Window &window);

    // Set text string in the clipboard buffer
    APPLIB_API void setClipboardString(const Window &window, const acul::string &text);

    // Initialize the library.
    APPLIB_API void initLibrary(acul::events::dispatcher *ed);

    // Destroy the library and release associated resources.
    APPLIB_API void destroyLibrary();
} // namespace awin

#endif