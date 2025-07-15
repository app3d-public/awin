/***********************************************************************************
 * App3D Window Library
 * A library for creating and managing 3D windows within the App3D environment.
 * ----------------------------------------------------------------------------------
 * Copyright (c) 2023, Wusiki Jeronii <wusikijeronii@gmail.com>
 ***********************************************************************************/

#ifndef APP_WINDOW_WINDOW_H
#define APP_WINDOW_WINDOW_H

#include <acul/event.hpp>
#include "acul/pair.hpp"
#include "types.hpp"

#define WINDOW_BACKEND_UNKNOWN -1
#define WINDOW_BACKEND_X11     0
#define WINDOW_BACKEND_WAYLAND 1
#define WINDOW_TIMEOUT_INF     -1
#define WINDOW_DONT_CARE       -1

namespace awin
{
    // A window entity in the windowing system
    class APPLIB_API Window
    {
    public:
        // Initialize a window with a title, dimensions, and creation flags.
        explicit Window(const acul::string &title, i32 width = WINDOW_DONT_CARE, i32 height = WINDOW_DONT_CARE,
                        WindowFlags flags = WINDOW_DEFAULT_FLAGS);

        // Destroy the window
        void destroy();

        // Get the window title
        acul::string title() const;

        // Set the window title
        void title(const acul::string &title);

        // Returns the width of the window.
        acul::point2D<i32> dimensions() const { return _data->dimenstions; }

        // Check if the window has decorations
        inline bool decorated() const { return (_data->flags & awin::WindowFlagBits::Decorated) != 0; }

        // Check if the window is resizable.
        inline bool resizable() const { return (_data->flags & awin::WindowFlagBits::Resizable) != 0; }

        // Check if the window is in fullscreen mode.
        bool fullscreen() const { return (_data->flags & awin::WindowFlagBits::Fullscreen) != 0; }

        // Enable fullscreen mode.
        void enable_fullscreen();

        // Disable fullscreen mode.
        void disable_fullscreen();

        // Get the current cursor position.
        acul::point2D<i32> cursor_position() const;

        // Set the cursor position
        void cursor_position(acul::point2D<i32> position);

        // Show the cursor.
        void show_cursor();

        // Hide the cursor.
        void hide_cursor();

        // Check if the cursor is hidden.
        inline bool is_cursor_hidden() const { return _data->is_cursor_hidden; }

        // Set cursor
        inline void set_cursor(Cursor *cursor) { _data->cursor = cursor; }

        // Check if the window is focused.
        inline bool focused() const { return _data->focused; }

        // Check if the window is minimized.
        inline bool minimized() const { return _data->flags & awin::WindowFlagBits::Minimized; }

        // Minimize the window
        void minimize();

        // Check if the window is maximized.
        inline bool maximized() const { return _data->flags & awin::WindowFlagBits::Maximized; }

        // Maximize the window
        void maximize();

        // Check if the window is hidden.
        inline bool hidden() const { return _data->flags & awin::WindowFlagBits::Hidden; }

        // Get the window's resize limits.
        inline acul::point2D<i32> resize_limit() const { return _data->resize_limit; }

        // Set the window's resize limits.
        void resize_limit(acul::point2D<i32> size)
        {
            _data->resize_limit = size;
            update_resize_limit();
        }

        // Check if the window is ready to be closed.
        inline bool ready_to_close() const { return _data->ready_to_close; }

        // Change the window's ready-to-close state.
        inline void ready_to_close(bool ready_to_close) { _data->ready_to_close = ready_to_close; }

        // Show the window if it is hidden.
        void show_window();

        // Hide the window
        void hide_window();

        // Get current window position
        acul::point2D<i32> position() const;

        // Set window position
        void position(acul::point2D<i32> position);

        // Center the window to the parent
        void center_window();

    private:
        WindowData *_data;

        void update_resize_limit();

        friend WindowData *get_window_data(const Window &);
    };

    // Updates the event registry by associating different types of window events with their listeners.
    // This function gathers listeners for various event types like mouse clicks, keyboard input,
    // focus changes, etc., and registers them to corresponding events. This allows the system
    // to efficiently dispatch events to the appropriate listeners as they occur.
    // Specific actions are taken for different platforms (e.g., Windows-specific event listeners)
    // to ensure compatibility and proper handling across different operating systems.
    APPLIB_API void update_events();

    // Data structures and platform-specific functions that are utilized by internal Window API and offer secure data
    // exchange with external API interfaces.
    namespace platform
    {
        // Initializes the platform-specific components and sets up the windowing system environment.
        bool init_platform();

        // Initializes the timer subsystem to enable precise time management for various window-related operations.
        void init_timer();

        // Retrieves the current time value from the timer subsystem, which is crucial for time-dependent calculations.
        u64 get_time_value();

        // Retrieves the timer frequency, an essential parameter for accurate time-based calculations.
        u64 get_time_frequency();

        // Performs cleanup and releases platform-specific resources associated with the windowing system environment.
        void destroy_platform();

        // A utility function responsible for processing keyboard input events specific to a particular window
        // implementation. It manages key presses, releases, and key modifiers, facilitating their propagation to the
        // appropriate event handlers.
        void input_key(WindowData *data, io::Key key, io::KeyPressState action, io::KeyMode mods);
    } // namespace platform

    // Events
    namespace event_id
    {
        enum : u64
        {
            Unknown = 0x0,
#ifdef _WIN32
            NCHitTest = 0x2D5AA1F9EE962269,
            NCMouseDown = 0x12D7ACB8440B7678,
#endif
            Focus = 0x05AC2ABF9E301AD1,
            CharInput = 0x0B37F6873EA5B017,
            KeyInput = 0x0E8A91707EFCEB90,
            MouseClick = 0x06254FC551B67986,
            MouseEnter = 0x1DFE0E9A4D85B1EE,
            MouseMove = 0x15F068FC45DB86CE,
            MouseMoveAbs = 0x037E8253212E7276,
            Scroll = 0x0D66A892FC053357,
            DpiChanged = 0x37516DB961C1BF7A,
            Minimize = 0x16AB16E6670A5AC2,
            Maximize = 0x0A8C9013D84CEC08,
            Resize = 0x1FB82ED0F4C701CB,
            Move = 0x2A5416AB994F5AAE
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
            : event(event_id::Focus), window(window), focused(focused)
        {
        }
    };

    // Represents a character input event in a window.
    struct CharInputEvent : public acul::events::event
    {
        awin::Window *window; // Pointer to the associated Window object.
        u32 charCode;         // Unicode character code.

        explicit CharInputEvent(awin::Window *window = nullptr, u32 charCode = 0)
            : event(event_id::CharInput), window(window), charCode(charCode)
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

        explicit KeyInputEvent(awin::Window *window = nullptr, io::Key key = io::Key::Unknown,
                               io::KeyPressState action = io::KeyPressState::Release, io::KeyMode mods = io::KeyMode{})
            : event(event_id::KeyInput), window(window), key(key), action(action), mods(mods)
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

        explicit MouseClickEvent(awin::Window *window = nullptr, io::MouseKey button = io::MouseKey::Unknown,
                                 io::KeyPressState action = io::KeyPressState::Release,
                                 io::KeyMode mods = io::KeyMode{})
            : event(event_id::MouseClick), window(window), button(button), action(action), mods(mods)
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
            : event(event_id::MouseEnter), window(window), entered(entered)
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
            : event(event_id::Scroll), window(window), h(hscroll), v(vscroll)
        {
        }
    };

    // Represents a DPI change event in a window.
    struct DpiChangedEvent : public acul::events::event
    {
        awin::Window *window; // Pointer to the associated Window object.
        acul::point2D<f32> dpi;

        explicit DpiChangedEvent(awin::Window *window = nullptr, f32 x_dpi = 0.0f, f32 y_dpi = 0.0f)
            : event(event_id::DpiChanged), window(window), dpi(x_dpi, y_dpi)
        {
        }
    };
    // Get the time elapsed since library initialization in seconds as a floating-point value.
    APPLIB_API f64 get_time();

    // Set the window library initialization time to the specified value in seconds.
    APPLIB_API void set_time(f64 time);

    /**
     * Retrieves information about the primary monitor of the system. The primary monitor
     * is typically the default display where applications are opened by default. This function
     * is particularly useful for applications that need to be aware of the monitor's
     * resolution, position, and size for layout optimizations, window positioning, or
     * adapting to different screen sizes.
     *
     * @return Position and size of the primary monitor.
     */
    APPLIB_API MonitorInfo get_primary_monitor_info();

    // Processes all pending events in the event queue. This function checks the state
    // of all windows and other event sources, processes those events, and returns
    // control after all events have been processed. Typically used in an application's
    // update loop to handle events as they occur.
    APPLIB_API void poll_events();

    // Waits for new events to occur and processes them as soon as they appear.
    // Unlike pollEvents, this function blocks the execution of the program until
    // new events are available. Useful in situations where you want to conserve CPU
    // usage when the application is idle or when you need to wait for user input
    // without continuously polling.
    APPLIB_API void wait_events();

    // Waits for events with a global timeout and processes them. If no events occur
    // within the given timeout period, the function returns. This is useful for
    // scenarios where you want to wait for events but also perform some other action
    // if no events occur within a certain time frame, such as updating the UI or
    // handling non-event-related logic.
    APPLIB_API void wait_events_timeout();

    // Pushes an empty event to the event queue.
    APPLIB_API void push_empty_event();

    // Retrieves the current dots per inch (DPI) value of the display.
    APPLIB_API f32 get_dpi(const Window &window);

    // Get the client area size
    APPLIB_API acul::point2D<i32> get_window_size(const Window &window);

    // Get text string from the clipboard buffer
    APPLIB_API acul::string get_clipboard_string(const Window &window);

    // Set text string in the clipboard buffer
    APPLIB_API void set_clipboard_string(const Window &window, const acul::string &text);

    // Initialize the library.
    APPLIB_API void init_library(acul::events::dispatcher *ed);

    // Destroy the library and release associated resources.
    APPLIB_API void destroy_library();

#ifndef _WIN32
    APPLIB_API void set_window_icon(Window &window, const acul::vector<Image> &images);
#endif
} // namespace awin

#endif