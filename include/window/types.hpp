#ifndef APP_WINDOW_TYPES_H
#define APP_WINDOW_TYPES_H

#include <core/std/basic_types.hpp>
#include <core/std/enum.hpp>

#define KEY_MOD_START_INDEX 106

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
        kCapsLock,
        kScrollLock,
        kNumLock,
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
        unknown = -1,
        left = 0,
        right = 1,
        middle = 2
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
    // Basic information about a monitor/display.
    struct MonitorInfo
    {
        int xpos;
        int ypos;
        int width;
        int height;
    };
    class Cursor
    {
        struct PlatformData;

    public:
        enum class Type
        {
            arrow,      // The regular arrow cursor.
            ibeam,      // The text input I-beam cursor.
            crosshair,  // The crosshair cursor.
            hand,       // The pointing hand cursor.
            resizeEW,   // The horizontal resize/move arrow cursor.  This is usually a horizontal double-headed arrow.
            resizeNS,   // The vertical resize/move cursor. This is usually a vertical double-headed arrow.
            resizeNWSE, // The top-left to bottom-right diagonal resize/move cursor.  This is usually a diagonal
                        // double-headed arrow.
            resizeNESW, // The top-right to bottom-left diagonal resize/move cursor.  This is usually a diagonal
                        // double-headed arrow.
            resizeAll,  // The omni-directional resize cursor/move.  This is usually either a combined horizontal and
                        // vertical double-headed arrow or a grabbing hand.
            notAllowed  // The operation-not-allowed shape.  This is usually a circle with a diagonal line through it.
        };

        explicit Cursor(PlatformData *platform = nullptr) : _platform(platform) {}

        Cursor(const Cursor &) = delete;
        Cursor &operator=(const Cursor &) = delete;

        Cursor &operator=(Cursor &&other) noexcept;

        ~Cursor();

        // Check if cursor is valid and was initialized.
        bool valid() const { return _platform != nullptr; }

        // Create a new cursor with the given type.
        static Cursor create(Type type);

        // Get the default cursor.
        static Cursor *defaultCursor();

        // Assign cursor to the platform context
        void assign();

    private:
        PlatformData *_platform;
    };

    // Flags for window creation, stored as u16 for memory efficiency.
    enum class CreationFlagsBits : u16
    {
        resizable = 0x0001,    // Allows window resizing.
        snapped = 0x0002,      // Enables window snapping to screen edges.
        decorated = 0x0004,    // Adds decorations like title bar and borders.
        fullscreen = 0x0008,   // Enables fullscreen mode.
        minimizebox = 0x00010, // Includes a minimize button.
        maximizebox = 0x00020, // Includes a maximize button.
        hidden = 0x00040,      // Does not show the window on creation.
        minimized = 0x00080,   // Starts minimized.
        maximized = 0x00100    // Starts maximized.
    };

    // Flags for window creation, stored as u8 for memory efficiency.
    using CreationFlags = Flags<CreationFlagsBits>;

#define WINDOW_DEFAULT_FLAGS                                                                         \
    CreationFlagsBits::resizable | CreationFlagsBits::minimizebox | CreationFlagsBits::maximizebox | \
        CreationFlagsBits::decorated | CreationFlagsBits::snapped
} // namespace window

template <>
struct FlagTraits<window::CreationFlagsBits>
{
    static constexpr bool isBitmask = true;
    static constexpr window::CreationFlags allFlags =
        (window::CreationFlagsBits::resizable | window::CreationFlagsBits::snapped |
         window::CreationFlagsBits::decorated | window::CreationFlagsBits::fullscreen |
         window::CreationFlagsBits::minimizebox | window::CreationFlagsBits::maximizebox |
         window::CreationFlagsBits::hidden | window::CreationFlagsBits::minimized |
         window::CreationFlagsBits::maximized);
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