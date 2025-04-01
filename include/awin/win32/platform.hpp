#ifndef APP_WINDOW_PLATFORM_WIN32_H
#define APP_WINDOW_PLATFORM_WIN32_H

#include <acul/api.hpp>
#include <acul/map.hpp>
#include <acul/point2d.hpp>
#include <acul/string/string.hpp>
#include <windows.h>
#include "../types.hpp"

namespace awin
{
    class Window;
    namespace platform
    {
        struct Win32PlatformData
        {
            acul::u16string title;
            DWORD style;
            DWORD exStyle;
            HWND hwnd;
            WCHAR highSurrogate;
            acul::point2D<i32> savedCursorPos{0, 0};
            bool cursorTracked{false};
            bool rawInput{false};
            LPBYTE rawInputData{nullptr};
            UINT rawInputSize{0};
        };

        using platform_data_t = Win32PlatformData;

        struct Win32AccessConnect
        {
            static APPLIB_API HWND getHWND(const Window &window);
        };

        using native_access = Win32AccessConnect;

        extern struct Win32Ctx
        {
            HINSTANCE instance;
            WNDCLASSEXW win32class;
            int frameX;
            int frameY;
            int padding;
            int screenWidth;
            int screenHeight;
            UINT dpi;
            acul::map<i16, io::Key> keymap{{VK_SPACE, io::Key::kSpace},
                                           {VK_OEM_7, io::Key::kApostroph},
                                           {VK_OEM_COMMA, io::Key::kComma},
                                           {VK_OEM_MINUS, io::Key::kMinus},
                                           {VK_OEM_PERIOD, io::Key::kPeriod},
                                           {VK_OEM_2, io::Key::kSlash},
                                           {0x30, io::Key::k0},
                                           {0x31, io::Key::k1},
                                           {0x32, io::Key::k2},
                                           {0x33, io::Key::k3},
                                           {0x34, io::Key::k4},
                                           {0x35, io::Key::k5},
                                           {0x36, io::Key::k6},
                                           {0x37, io::Key::k7},
                                           {0x38, io::Key::k8},
                                           {0x39, io::Key::k9},
                                           {VK_OEM_1, io::Key::kSemicolon},
                                           {VK_OEM_PLUS, io::Key::kEqual},
                                           {0x41, io::Key::kA},
                                           {0x42, io::Key::kB},
                                           {0x43, io::Key::kC},
                                           {0x44, io::Key::kD},
                                           {0x45, io::Key::kE},
                                           {0x46, io::Key::kF},
                                           {0x47, io::Key::kG},
                                           {0x48, io::Key::kH},
                                           {0x49, io::Key::kI},
                                           {0x4A, io::Key::kJ},
                                           {0x4B, io::Key::kK},
                                           {0x4C, io::Key::kL},
                                           {0x4D, io::Key::kM},
                                           {0x4E, io::Key::kN},
                                           {0x4F, io::Key::kO},
                                           {0x50, io::Key::kP},
                                           {0x51, io::Key::kQ},
                                           {0x52, io::Key::kR},
                                           {0x53, io::Key::kS},
                                           {0x54, io::Key::kT},
                                           {0x55, io::Key::kU},
                                           {0x56, io::Key::kV},
                                           {0x57, io::Key::kW},
                                           {0x58, io::Key::kX},
                                           {0x59, io::Key::kY},
                                           {0x5A, io::Key::kZ},
                                           {VK_OEM_4, io::Key::kLeftBrace},
                                           {VK_OEM_5, io::Key::kBackslash},
                                           {VK_OEM_6, io::Key::kRightBrace},
                                           {VK_OEM_3, io::Key::kGraveAccent},
                                           {VK_ESCAPE, io::Key::kEscape},
                                           {VK_RETURN, io::Key::kEnter},
                                           {VK_TAB, io::Key::kTab},
                                           {VK_BACK, io::Key::kBackspace},
                                           {VK_INSERT, io::Key::kInsert},
                                           {VK_DELETE, io::Key::kDelete},
                                           {VK_RIGHT, io::Key::kRight},
                                           {VK_LEFT, io::Key::kLeft},
                                           {VK_DOWN, io::Key::kDown},
                                           {VK_UP, io::Key::kUp},
                                           {VK_PRIOR, io::Key::kPageUp},
                                           {VK_NEXT, io::Key::kPageDown},
                                           {VK_HOME, io::Key::kHome},
                                           {VK_END, io::Key::kEnd},
                                           {VK_CAPITAL, io::Key::kCapsLock},
                                           {VK_SCROLL, io::Key::kScrollLock},
                                           {VK_NUMLOCK, io::Key::kNumLock},
                                           {VK_SNAPSHOT, io::Key::kPrintScreen},
                                           {VK_PAUSE, io::Key::kPause},
                                           {VK_F1, io::Key::kF1},
                                           {VK_F2, io::Key::kF2},
                                           {VK_F3, io::Key::kF3},
                                           {VK_F4, io::Key::kF4},
                                           {VK_F5, io::Key::kF5},
                                           {VK_F6, io::Key::kF6},
                                           {VK_F7, io::Key::kF7},
                                           {VK_F8, io::Key::kF8},
                                           {VK_F9, io::Key::kF9},
                                           {VK_F10, io::Key::kF10},
                                           {VK_F11, io::Key::kF11},
                                           {VK_F12, io::Key::kF12},
                                           {VK_F13, io::Key::kF13},
                                           {VK_F14, io::Key::kF14},
                                           {VK_F15, io::Key::kF15},
                                           {VK_F16, io::Key::kF16},
                                           {VK_F17, io::Key::kF17},
                                           {VK_F18, io::Key::kF18},
                                           {VK_F19, io::Key::kF19},
                                           {VK_F20, io::Key::kF20},
                                           {VK_F21, io::Key::kF21},
                                           {VK_F22, io::Key::kF22},
                                           {VK_F23, io::Key::kF23},
                                           {VK_F24, io::Key::kF24},
                                           {VK_NUMPAD0, io::Key::kKP0},
                                           {VK_NUMPAD1, io::Key::kKP1},
                                           {VK_NUMPAD2, io::Key::kKP2},
                                           {VK_NUMPAD3, io::Key::kKP3},
                                           {VK_NUMPAD4, io::Key::kKP4},
                                           {VK_NUMPAD5, io::Key::kKP5},
                                           {VK_NUMPAD6, io::Key::kKP6},
                                           {VK_NUMPAD7, io::Key::kKP7},
                                           {VK_NUMPAD8, io::Key::kKP8},
                                           {VK_NUMPAD9, io::Key::kKP9},
                                           {VK_DECIMAL, io::Key::kKPDecimal},
                                           {VK_DIVIDE, io::Key::kKPDivide},
                                           {VK_MULTIPLY, io::Key::kKPMultiply},
                                           {VK_SUBTRACT, io::Key::kKPSubtract},
                                           {VK_ADD, io::Key::kKPAdd},
                                           {VK_RETURN, io::Key::kKPEnter},
                                           {VK_OEM_PLUS, io::Key::kKPEqual},
                                           {VK_LSHIFT, io::Key::kLeftShift},
                                           {VK_LCONTROL, io::Key::kLeftControl},
                                           {VK_LMENU, io::Key::kLeftAlt},
                                           {VK_LWIN, io::Key::kLeftSuper},
                                           {VK_RSHIFT, io::Key::kRightShift},
                                           {VK_RCONTROL, io::Key::kRightControl},
                                           {VK_RMENU, io::Key::kRightAlt},
                                           {VK_RWIN, io::Key::kRightSuper},
                                           {VK_APPS, io::Key::kMenu}};
        } ctx;
    } // namespace platform
} // namespace awin

#endif