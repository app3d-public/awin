#ifndef APP_WINDOW_TYPES_H
#define APP_WINDOW_TYPES_H

#include <acul/api.hpp>
#include <acul/enum.hpp>
#include <acul/memory/alloc.hpp>
#include <acul/pair.hpp>
#include <acul/scalars.hpp>

#define KEY_MOD_START_INDEX 106

// Keys
namespace awin
{
    class Window;

    namespace io
    {
        enum class Key : i16
        {
            unknown,
            space,
            apostroph,
            comma,
            minus,
            period,
            slash,
            d0,
            d1,
            d2,
            d3,
            d4,
            d5,
            d6,
            d7,
            d8,
            d9,
            semicolon,
            equal,
            a,
            b,
            c,
            d,
            e,
            f,
            g,
            h,
            i,
            j,
            k,
            l,
            m,
            n,
            o,
            p,
            q,
            r,
            s,
            t,
            u,
            v,
            w,
            x,
            y,
            z,
            lbrace,
            backslash,
            rbrace,
            grave_accent,
            escape,
            enter,
            tab,
            backspace,
            insert,
            del,
            right,
            left,
            down,
            up,
            page_up,
            page_down,
            home,
            end,
            print_screen,
            pause,
            f1,
            f2,
            f3,
            f4,
            f5,
            f6,
            f7,
            f8,
            f9,
            f10,
            f11,
            f12,
            f13,
            f14,
            f15,
            f16,
            f17,
            f18,
            f19,
            f20,
            f21,
            f22,
            f23,
            f24,
            kp_0,
            kp_1,
            kp_2,
            kp_3,
            kp_4,
            kp_5,
            kp_6,
            kp_7,
            kp_8,
            kp_9,
            kp_decimal,
            kp_divide,
            kp_multiply,
            kp_subtract,
            kp_add,
            kp_enter,
            kp_equal,
            caps_lock,
            scroll_lock,
            num_lock,
            lshift,
            lcontrol,
            lalt,
            lsuper,
            rshift,
            rcontrol,
            ralt,
            rsuper,
            menu,
            last = menu
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

        struct KeyModeBits
        {
            enum enum_type : i8
            {
                shift = 0x0001,
                control = 0x0002,
                alt = 0x0004,
                super = 0x0008,
                caps_lock = 0x0010,
                num_lock = 0x0020
            };
            using flag_bitmask = std::true_type;
        };

        using KeyMode = acul::flags<KeyModeBits>;
    } // namespace io

    // Basic information about a monitor/display.
    struct MonitorInfo
    {
        acul::point2D<long> work;
        acul::point2D<long> dimensions;
    };

    class APPLIB_API Cursor
    {

    public:
        struct Platform;

        enum class Type
        {
            arrow,       // The regular arrow cursor.
            ibeam,       // The text input I-beam cursor.
            crosshair,   // The crosshair cursor.
            hand,        // The pointing hand cursor.
            resize_ew,   // The horizontal resize/move arrow cursor.  This is usually a horizontal double-headed arrow.
            resize_ns,   // The vertical resize/move cursor. This is usually a vertical double-headed arrow.
            resize_nwse, // The top-left to bottom-right diagonal resize/move cursor.  This is usually a diagonal
                         // double-headed arrow.
            resize_nesw, // The top-right to bottom-left diagonal resize/move cursor.  This is usually a diagonal
                         // double-headed arrow.
            resize_all,  // The omni-directional resize cursor/move.  This is usually either a combined horizontal and
                         // vertical double-headed arrow or a grabbing hand.
            not_allowed  // The operation-not-allowed shape.  This is usually a circle with a diagonal line through it.
        };

        Cursor(Platform *pd = NULL) : _pd(pd) {}

        Cursor(const Cursor &) = delete;
        Cursor &operator=(const Cursor &) = delete;

        Cursor &operator=(Cursor &&other) noexcept;

        // Check if cursor is valid and was initialized.
        bool valid() const;

        // Create a new cursor with the given type.
        static Cursor create(Type type);

        // Assign cursor to the platform context
        void assign(Window *window);

        void reset()
        {
            acul::mem_allocator<Platform>::deallocate(_pd);
            _pd = nullptr;
        }

    private:
        Platform *_pd;

        friend Cursor::Platform *get_cursor_pd(Cursor *cursor);
    };

    // Flags for window creation, stored as u16 for memory efficiency.
    struct WindowFlagBits
    {
#ifdef _WIN32
    #define NATIVE_RESERVE_FLAG snapped
#else
    #define NATIVE_RESERVE_FLAG activated
#endif
        enum enum_type : u16
        {
            resizable = 0x0001,           // Allows window resizing.
            NATIVE_RESERVE_FLAG = 0x0002, //
                                          // Win32: Enables window snapping to screen edges.
                                          // Wayland: Reserved for internal API.
                                          // Actually doesn't need in a Window instance
            decorated = 0x0004,           // Adds decorations like title bar and borders.
            fullscreen = 0x0008,          // Enables fullscreen mode.
            minimize_box = 0x00010,       // Includes a minimize button.
            maximize_box = 0x00020,       // Includes a maximize button.
            hidden = 0x00040,             // Does not show the window on creation.
            minimized = 0x00080,          // Minimized.
            maximized = 0x00100,          // Maximized.
        };
        using flag_bitmask = std::true_type;
    };

    // Flags for window creation, stored as u8 for memory efficiency.
    using WindowFlags = acul::flags<WindowFlagBits>;

#define WINDOW_DEFAULT_FLAGS                                                                  \
    WindowFlagBits::resizable | WindowFlagBits::minimize_box | WindowFlagBits::maximize_box | \
        WindowFlagBits::decorated | WindowFlagBits::NATIVE_RESERVE_FLAG

    struct Image
    {
        acul::point2D<int> dimenstions;
        const void *pixels;
    };

    struct WindowData
    {
        Window *owner;
        acul::point2D<i32> dimenstions;
        WindowFlags flags;
        bool is_cursor_hidden{false};
        bool focused{false};
        bool ready_to_close = false;
        acul::point2D<i32> resize_limit{0, 0};
        io::KeyPressState keys[io::Key::last + 1];
        Cursor *cursor{NULL};
    };
} // namespace awin
#endif