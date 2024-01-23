#ifndef APP_WINDOW_TYPES_H
#define APP_WINDOW_TYPES_H

#include <core/event/event.hpp>
#include <core/std/array.hpp>
#include <core/std/basic_types.hpp>
#include <core/std/enum.hpp>
#include <memory>

#ifdef _WIN32
    #include <windows.h>
#endif

// Keys
namespace io
{
    enum class Key : i16
    {
        kUnknown,
        kSpace,
        kApostroph,
        kComma,
        kMinus,
        kPeriod,
        kSlash,
        k0,
        k1,
        k2,
        k3,
        k4,
        k5,
        k6,
        k7,
        k8,
        k9,
        kSemicolon,
        kEqual,
        kA,
        kB,
        kC,
        kD,
        kE,
        kF,
        kG,
        kH,
        kI,
        kJ,
        kK,
        kL,
        kM,
        kN,
        kO,
        kP,
        kQ,
        kR,
        kS,
        kT,
        kU,
        kV,
        kW,
        kX,
        kY,
        kZ,
        kLeftBrace,
        kBackslash,
        kRightBrace,
        kGraveAccent,
        kEscape,
        kEnter,
        kTab,
        kBackspace,
        kInsert,
        kDelete,
        kRight,
        kLeft,
        kDown,
        kUp,
        kPageUp,
        kPageDown,
        kHome,
        kEnd,
        kCapsLock,
        kScrollLock,
        kNumLock,
        kPrintScreen,
        kPause,
        kF1,
        kF2,
        kF3,
        kF4,
        kF5,
        kF6,
        kF7,
        kF8,
        kF9,
        kF10,
        kF11,
        kF12,
        kF13,
        kF14,
        kF15,
        kF16,
        kF17,
        kF18,
        kF19,
        kF20,
        kF21,
        kF22,
        kF23,
        kF24,
        kKP0,
        kKP1,
        kKP2,
        kKP3,
        kKP4,
        kKP5,
        kKP6,
        kKP7,
        kKP8,
        kKP9,
        kKPDecimal,
        kKPDivide,
        kKPMultiply,
        kKPSubtract,
        kKPAdd,
        kKPEnter,
        kKPEqual,
        kLeftShift,
        kLeftControl,
        kLeftAlt,
        kLeftSuper,
        kRightShift,
        kRightControl,
        kRightAlt,
        kRightSuper,
        kMenu,
        kLast = kMenu
    };

    inline constexpr i16 operator+(Key k) { return static_cast<i16>(k); }
    inline constexpr i16 operator+(Key lhs, i16 rhs) { return static_cast<i16>(lhs) + rhs; }

    enum class MouseKey
    {
        unknown,
        left,
        right,
        middle
    };

    inline constexpr i16 operator+(MouseKey k) { return static_cast<i16>(k); }
    inline constexpr i16 operator+(MouseKey lhs, i16 rhs) { return static_cast<i16>(lhs) + rhs; }

    enum class KeyPressState : i8
    {
        release,
        press,
        repeat
    };

    enum class KeyModeBits : i8
    {
        shift = 0x0001,
        control = 0x0002,
        alt = 0x0004,
        super = 0x0008,
        capsLock = 0x0010,
        numLock = 0x0020
    };

    using KeyMode = Flags<KeyModeBits>;
} // namespace io

namespace window
{
    // Flags for window creation, stored as u8 for memory efficiency.
    enum class CreationFlagsBits : u8
    {
        resizable = 0x0001,    // Allows window resizing.
        snapped = 0x0002,      // Enables window snapping to screen edges.
        decorated = 0x0004,    // Adds decorations like title bar and borders.
        fullscreen = 0x0008,   // Enables fullscreen mode.
        minimizebox = 0x00010, // Includes a minimize button.
        maximizebox = 0x00020, // Includes a maximize button.
        hidden = 0x00040       // Does not show the window on creation.
    };

#define WINDOW_DEFAULT_FLAGS                                                                         \
    CreationFlagsBits::resizable | CreationFlagsBits::minimizebox | CreationFlagsBits::maximizebox | \
        CreationFlagsBits::decorated | CreationFlagsBits::snapped

    // Flags for window creation, stored as u8 for memory efficiency.
    using CreationFlags = Flags<CreationFlagsBits>;

    // Basic information about a monitor/display.
    struct MonitorInfo
    {
        int xpos;
        int ypos;
        int width;
        int height;
    };

    // Abstract base class for platform-specific window classes like Win32Window, X11Window, etc.
    class WindowBase
    {
    public:
        // Initialize a window with a title, dimensions, and creation flags.
        WindowBase(const std::string &title, i32 width, i32 height, window::CreationFlags flags)
            : _dimenstions(width, height), _flags(flags)
        {
        }

        virtual ~WindowBase() = default;

        // Get the window title
        virtual std::string title() const = 0;

        // Set the window title
        virtual void title(const std::string &title) = 0;

        // Returns the width of the window.
        Point2D dimensions() const { return _dimenstions; }

        // Check if the window has decorations
        bool decorated() const { return (_flags & window::CreationFlagsBits::decorated) != 0; }

        // Check if the window is resizable.
        bool resizable() const { return (_flags & window::CreationFlagsBits::resizable) != 0; }

        // Check if the window is in fullscreen mode.
        bool fullscreen() const { return (_flags & window::CreationFlagsBits::fullscreen) != 0; }

        // Enable fullscreen mode.
        virtual void enableFullscreen() = 0;

        // Disable fullscreen mode.
        virtual void disableFullscreen() = 0;

        // Get the current cursor position.
        virtual Point2D cursorPosition() const = 0;

        // Set the cursor position
        virtual void cursorPosition(Point2D position) = 0;

        // Show the cursor.
        virtual void showCursor() = 0;

        // Hide the cursor.
        virtual void hideCursor() = 0;

        // Check if the window is focused.
        bool focused() const { return _focused; }

        // Check if the window is minimized.
        bool minimized() const { return _minimized; }

        // Check if the window is maximized.
        bool maximized() const { return _maximized; }

        // Check if the window is hidden.
        bool hidden() const { return (_flags & window::CreationFlagsBits::hidden) != 0; }

        // Get the window's resize limits.
        Point2D resizeLimit() const { return _resizeLimit; }

        // Set the window's resize limits.
        void resizeLimit(i32 width, i32 height) { _resizeLimit = {width, height}; }

        // Check if the window is ready to be closed.
        bool readyToClose() const { return _readyToClose; }

        // Show the window if it is hidden.
        virtual void showWindow() = 0;

        // Hide the window
        virtual void hideWindow() = 0;

        // Get current window position
        virtual Point2D windowPos() const = 0;

        // Set window position
        virtual void windowPos(Point2D position) = 0;

        // Center the window to the parent
        virtual void centerWindowPos() = 0;

    protected:
        Point2D _dimenstions;
        window::CreationFlags _flags;
        bool _isCursorHidden = false;
        bool _focused;
        bool _minimized;
        bool _maximized;
        bool _readyToClose = false;
        Point2D _resizeLimit{0, 0};
        io::KeyPressState _keys[io::Key::kLast + 1];

        void inputKey(io::Key key, io::KeyPressState action, io::KeyMode mods);
    };
} // namespace window

// Events
#ifdef _WIN32
// Event specific to Win32 platform, carrying native window message data.
struct Win32NativeEvent : public Event
{
    window::WindowBase *window; // Pointer to the associated Window object.
    UINT uMsg;                  // Windows message identifier.
    WPARAM wParam;              // Additional message information.
    LPARAM lParam;              // Additional message information.
    LRESULT *lResult;           // Pointer to the result of the message processing.

    Win32NativeEvent(const std::string &name = "", window::WindowBase *window = nullptr, UINT uMsg = 0,
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
    window::WindowBase *window;

    // Whether the window is focused or not.
    bool focused;

    FocusEvent(const std::string &name = "", window::WindowBase *window = nullptr, bool focused = false)
        : Event(name), window(window), focused(focused)
    {
    }
};

// Represents a character input event in a window.
struct CharInputEvent : public Event
{
    window::WindowBase *window; // Pointer to the associated Window object.
    u32 charCode;               // Unicode character code.

    CharInputEvent(const std::string &name = "", window::WindowBase *window = nullptr, u32 charCode = 0)
        : Event(name), window(window), charCode(charCode)
    {
    }
};

// Represents a keyboard input event in a window.
struct KeyInputEvent : public Event
{
    window::WindowBase *window; // Pointer to the associated Window object.
    io::Key key;                // The key involved in the event.
    io::KeyPressState action;   // The action (press, release, repeat) associated with the key.
    io::KeyMode mods;           // The key modifiers associated with the key.

    KeyInputEvent(const std::string &name = "", window::WindowBase *window = nullptr, io::Key key = io::Key::kUnknown,
                  io::KeyPressState action = io::KeyPressState::release, io::KeyMode mods = io::KeyMode{})
        : Event(name), window(window), key(key), action(action), mods(mods)
    {
    }
};

// Represents a mouse click event in a window.
struct MouseClickEvent : public Event
{
    window::WindowBase *window; // Pointer to the associated Window object.
    io::MouseKey button;        // The mouse button involved in the event.
    io::KeyPressState action;   // The action (press, release, repeat) associated with the key.
    io::KeyMode mods;           // The key modifiers associated with the key.

    MouseClickEvent(const std::string &name = "", window::WindowBase *window = nullptr,
                    io::MouseKey button = io::MouseKey::unknown, io::KeyPressState action = io::KeyPressState::release,
                    io::KeyMode mods = io::KeyMode{})
        : Event(name), window(window), button(button), action(action), mods(mods)
    {
    }
};

// Represents a mouse position event in a window. This one is emitted when the mouse enters or leaves the window.
struct CursorEnterEvent : public Event
{
    window::WindowBase *window; // Pointer to the associated Window object.
    bool entered;               // Whether the mouse entered or left the window.

    CursorEnterEvent(const std::string &name = "", window::WindowBase *window = nullptr, bool entered = false)
        : Event(name), window(window), entered(entered)
    {
    }
};

// Represents a position change event in a window.
struct PosEvent : public Event
{
    window::WindowBase *window; // Pointer to the associated Window object.
    Point2D position;           // The new position.

    PosEvent(const std::string &name = "", window::WindowBase *window = nullptr, Point2D position = Point2D())
        : Event(name), window(window), position(position)
    {
    }
};

// Represents a scroll event in a window.
struct ScrollEvent : public Event
{
    window::WindowBase *window; // Pointer to the associated Window object.
    f32 offset;                 // The scroll offset.

    ScrollEvent(const std::string &name = "", window::WindowBase *window = nullptr, f32 offset = 0.0f)
        : Event(name), window(window), offset(offset)
    {
    }
};

// Represents a DPI change event in a window.
struct DpiChangedEvent : public Event
{
    window::WindowBase *window; // Pointer to the associated Window object.
    f32 xDpi;                   // New DPI value in the x-axis.
    f32 yDpi;                   // New DPI value in the y-axis.

    DpiChangedEvent(const std::string &name = "", window::WindowBase *window = nullptr, f32 xDpi = 0.0f,
                    f32 yDpi = 0.0f)
        : Event(name), window(window), xDpi(xDpi), yDpi(yDpi)
    {
    }
};

namespace window
{
    namespace _internal
    {
        extern struct WindowEvents
        {
            EventManager *e;
#ifdef _WIN32
            EventListener<Win32NativeEvent> *NCLMouseClick;
            EventListener<Win32NativeEvent> *NCHitTest;
#endif
            Array<std::shared_ptr<EventListener<FocusEvent>>> focusEvents;
            Array<std::shared_ptr<EventListener<CharInputEvent>>> charInputEvents;
            Array<std::shared_ptr<EventListener<KeyInputEvent>>> keyInputEvents;
            Array<std::shared_ptr<EventListener<MouseClickEvent>>> mouseClickEvents;
            Array<std::shared_ptr<EventListener<CursorEnterEvent>>> cursorEnterEvents;
            Array<std::shared_ptr<EventListener<PosEvent>>> cursorPosEvents;
            Array<std::shared_ptr<EventListener<ScrollEvent>>> scrollEvents;
            Array<std::shared_ptr<EventListener<PosEvent>>> minimizeEvents;
            Array<std::shared_ptr<EventListener<PosEvent>>> maximizeEvents;
            Array<std::shared_ptr<EventListener<PosEvent>>> resizeEvents;
            Array<std::shared_ptr<EventListener<PosEvent>>> moveEvents;
            Array<std::shared_ptr<EventListener<DpiChangedEvent>>> dpiChangedEvents;

        } gWindowEvents;

        template <typename T, typename... Args>
        inline void emitWindowEvent(const Array<std::shared_ptr<EventListener<T>>> &listener, Args &&...args)
        {
            T event(std::forward<Args>(args)...);
            for (const auto &l : listener)
                l->invoke(event);
        }
    } // namespace _internal
} // namespace window

template <>
struct FlagTraits<window::CreationFlagsBits>
{
    static constexpr bool isBitmask = true;
    static constexpr window::CreationFlags allFlags =
        (window::CreationFlagsBits::resizable | window::CreationFlagsBits::snapped |
         window::CreationFlagsBits::decorated | window::CreationFlagsBits::fullscreen |
         window::CreationFlagsBits::minimizebox | window::CreationFlagsBits::maximizebox);
};

template <>
struct FlagTraits<io::KeyModeBits>
{
    static constexpr bool isBitmask = true;
    static constexpr io::KeyMode allFlags = io::KeyModeBits::shift | io::KeyModeBits::control | io::KeyModeBits::alt |
                                            io::KeyModeBits::super | io::KeyModeBits::capsLock |
                                            io::KeyModeBits::numLock;
};

#endif