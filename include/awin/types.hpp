#ifndef APP_WINDOW_TYPES_H
#define APP_WINDOW_TYPES_H

#include <acul/api.hpp>
#include <acul/enum.hpp>
#include <acul/scalars.hpp>

#ifdef _WIN32
    #include "win32/cursor.hpp"
#else
    #include "linux/cursor.hpp"
#endif

#define KEY_MOD_START_INDEX 106

// Keys
namespace awin
{
    namespace io
    {
        enum class Key : i16
        {
            Unknown,
            Space,
            Apostroph,
            Comma,
            Minus,
            Period,
            Slash,
            D0,
            D1,
            D2,
            D3,
            D4,
            D5,
            D6,
            D7,
            D8,
            D9,
            Semicolon,
            Equal,
            A,
            B,
            C,
            D,
            E,
            F,
            G,
            H,
            I,
            J,
            K,
            L,
            M,
            N,
            O,
            P,
            Q,
            R,
            S,
            T,
            U,
            V,
            W,
            X,
            Y,
            Z,
            LeftBrace,
            Backslash,
            RightBrace,
            GraveAccent,
            Escape,
            Enter,
            Tab,
            Backspace,
            Insert,
            Delete,
            Right,
            Left,
            Down,
            Up,
            PageUp,
            PageDown,
            Home,
            End,
            PrintScreen,
            Pause,
            F1,
            F2,
            F3,
            F4,
            F5,
            F6,
            F7,
            F8,
            F9,
            F10,
            F11,
            F12,
            F13,
            F14,
            F15,
            F16,
            F17,
            F18,
            F19,
            F20,
            F21,
            F22,
            F23,
            F24,
            KP0,
            KP1,
            KP2,
            KP3,
            KP4,
            KP5,
            KP6,
            KP7,
            KP8,
            KP9,
            KPDecimal,
            KPDivide,
            KPMultiply,
            KPSubtract,
            KPAdd,
            KPEnter,
            KPEqual,
            CapsLock,
            ScrollLock,
            NumLock,
            LeftShift,
            LeftControl,
            LeftAlt,
            LeftSuper,
            RightShift,
            RightControl,
            RightAlt,
            RightSuper,
            Menu,
            Last = Menu
        };

        inline constexpr i16 operator+(Key k) { return static_cast<i16>(k); }
        inline constexpr i16 operator+(Key lhs, i16 rhs) { return static_cast<i16>(lhs) + rhs; }

        enum class MouseKey
        {
            Unknown = -1,
            Left = 0,
            Right = 1,
            Middle = 2
        };

        inline constexpr i16 operator+(MouseKey k) { return static_cast<i16>(k); }
        inline constexpr i16 operator+(MouseKey lhs, i16 rhs) { return static_cast<i16>(lhs) + rhs; }

        enum class KeyPressState : i8
        {
            Release,
            Press,
            Repeat
        };

        struct KeyModeBits
        {
            enum enum_type : i8
            {
                Shift = 0x0001,
                Control = 0x0002,
                Alt = 0x0004,
                Super = 0x0008,
                CapsLock = 0x0010,
                NumLock = 0x0020
            };
            using flag_bitmask = std::true_type;
        };

        using KeyMode = acul::flags<KeyModeBits>;
    } // namespace io

    // Basic information about a monitor/display.
    struct MonitorInfo
    {
        int xpos;
        int ypos;
        int width;
        int height;
    };

    class APPLIB_API Cursor
    {
    public:
        enum class Type
        {
            Arrow,      // The regular arrow cursor.
            Ibeam,      // The text input I-beam cursor.
            Crosshair,  // The crosshair cursor.
            Hand,       // The pointing hand cursor.
            ResizeEW,   // The horizontal resize/move arrow cursor.  This is usually a horizontal double-headed arrow.
            ResizeNS,   // The vertical resize/move cursor. This is usually a vertical double-headed arrow.
            ResizeNWSE, // The top-left to bottom-right diagonal resize/move cursor.  This is usually a diagonal
                        // double-headed arrow.
            ResizeNESW, // The top-right to bottom-left diagonal resize/move cursor.  This is usually a diagonal
                        // double-headed arrow.
            ResizeAll,  // The omni-directional resize cursor/move.  This is usually either a combined horizontal and
                        // vertical double-headed arrow or a grabbing hand.
            NotAllowed  // The operation-not-allowed shape.  This is usually a circle with a diagonal line through it.
        };

        Cursor(const platform::native_cursor_t &cursor = platform::native_cursor_t()) : _platform(cursor) {}

        Cursor(const Cursor &) = delete;
        Cursor &operator=(const Cursor &) = delete;

        Cursor &operator=(Cursor &&other) noexcept;

        // Check if cursor is valid and was initialized.
        bool valid() const { return _platform.valid(); }

        // Create a new cursor with the given type.
        static Cursor create(Type type);

        // Get the default cursor.
        static Cursor *default_cursor();

        // Assign cursor to the platform context
        void assign();

    private:
        platform::native_cursor_t _platform;
    };

    // Flags for window creation, stored as u16 for memory efficiency.
    struct CreationFlagsBits
    {
        enum enum_type : u16
        {
            Resizable = 0x0001,    // Allows window resizing.
            Snapped = 0x0002,      // Enables window snapping to screen edges.
            Decorated = 0x0004,    // Adds decorations like title bar and borders.
            Fullscreen = 0x0008,   // Enables fullscreen mode.
            MinimizeBox = 0x00010, // Includes a minimize button.
            MaximizeBox = 0x00020, // Includes a maximize button.
            Hidden = 0x00040,      // Does not show the window on creation.
            Minimized = 0x00080,   // Starts minimized.
            Maximized = 0x00100,   // Starts maximized.
        };
        using flag_bitmask = std::true_type;
    };

    // Flags for window creation, stored as u8 for memory efficiency.
    using CreationFlags = acul::flags<CreationFlagsBits>;

#define WINDOW_DEFAULT_FLAGS                                                                         \
    CreationFlagsBits::Resizable | CreationFlagsBits::MinimizeBox | CreationFlagsBits::MaximizeBox | \
        CreationFlagsBits::Decorated | CreationFlagsBits::Snapped
} // namespace awin
#endif