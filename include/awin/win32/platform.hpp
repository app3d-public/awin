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
            DWORD ex_style;
            HWND hwnd;
            WCHAR high_surrogate;
            acul::point2D<i32> saved_cursor_pos{0, 0};
            bool cursor_tracked{false};
            bool raw_input{false};
            LPBYTE raw_input_data{nullptr};
            UINT raw_input_size{0};
        };

        using platform_data_t = Win32PlatformData;

        struct Win32AccessConnect
        {
            static APPLIB_API HWND get_hwnd(const Window &window);

            static APPLIB_API acul::point2D<i32> get_full_client_size(const Window &window);
        };

        using native_access = Win32AccessConnect;

        extern struct Win32Ctx
        {
            HINSTANCE instance;
            WNDCLASSEXW win32_class;
            DWORD thread_id;
            int padding;
            acul::point2D<int> frame, screen;
            UINT dpi;
            acul::map<i16, io::Key> keymap{{VK_SPACE, io::Key::Space},
                                           {VK_OEM_7, io::Key::Apostroph},
                                           {VK_OEM_COMMA, io::Key::Comma},
                                           {VK_OEM_MINUS, io::Key::Minus},
                                           {VK_OEM_PERIOD, io::Key::Period},
                                           {VK_OEM_2, io::Key::Slash},
                                           {0x30, io::Key::D0},
                                           {0x31, io::Key::D1},
                                           {0x32, io::Key::D2},
                                           {0x33, io::Key::D3},
                                           {0x34, io::Key::D4},
                                           {0x35, io::Key::D5},
                                           {0x36, io::Key::D6},
                                           {0x37, io::Key::D7},
                                           {0x38, io::Key::D8},
                                           {0x39, io::Key::D9},
                                           {VK_OEM_1, io::Key::Semicolon},
                                           {VK_OEM_PLUS, io::Key::Equal},
                                           {0x41, io::Key::A},
                                           {0x42, io::Key::B},
                                           {0x43, io::Key::C},
                                           {0x44, io::Key::D},
                                           {0x45, io::Key::E},
                                           {0x46, io::Key::F},
                                           {0x47, io::Key::G},
                                           {0x48, io::Key::H},
                                           {0x49, io::Key::I},
                                           {0x4A, io::Key::J},
                                           {0x4B, io::Key::K},
                                           {0x4C, io::Key::L},
                                           {0x4D, io::Key::M},
                                           {0x4E, io::Key::N},
                                           {0x4F, io::Key::O},
                                           {0x50, io::Key::P},
                                           {0x51, io::Key::Q},
                                           {0x52, io::Key::R},
                                           {0x53, io::Key::S},
                                           {0x54, io::Key::T},
                                           {0x55, io::Key::U},
                                           {0x56, io::Key::V},
                                           {0x57, io::Key::W},
                                           {0x58, io::Key::X},
                                           {0x59, io::Key::Y},
                                           {0x5A, io::Key::Z},
                                           {VK_OEM_4, io::Key::LeftBrace},
                                           {VK_OEM_5, io::Key::Backslash},
                                           {VK_OEM_6, io::Key::RightBrace},
                                           {VK_OEM_3, io::Key::GraveAccent},
                                           {VK_ESCAPE, io::Key::Escape},
                                           {VK_RETURN, io::Key::Enter},
                                           {VK_TAB, io::Key::Tab},
                                           {VK_BACK, io::Key::Backspace},
                                           {VK_INSERT, io::Key::Insert},
                                           {VK_DELETE, io::Key::Delete},
                                           {VK_RIGHT, io::Key::Right},
                                           {VK_LEFT, io::Key::Left},
                                           {VK_DOWN, io::Key::Down},
                                           {VK_UP, io::Key::Up},
                                           {VK_PRIOR, io::Key::PageUp},
                                           {VK_NEXT, io::Key::PageDown},
                                           {VK_HOME, io::Key::Home},
                                           {VK_END, io::Key::End},
                                           {VK_CAPITAL, io::Key::CapsLock},
                                           {VK_SCROLL, io::Key::ScrollLock},
                                           {VK_NUMLOCK, io::Key::NumLock},
                                           {VK_SNAPSHOT, io::Key::PrintScreen},
                                           {VK_PAUSE, io::Key::Pause},
                                           {VK_F1, io::Key::F1},
                                           {VK_F2, io::Key::F2},
                                           {VK_F3, io::Key::F3},
                                           {VK_F4, io::Key::F4},
                                           {VK_F5, io::Key::F5},
                                           {VK_F6, io::Key::F6},
                                           {VK_F7, io::Key::F7},
                                           {VK_F8, io::Key::F8},
                                           {VK_F9, io::Key::F9},
                                           {VK_F10, io::Key::F10},
                                           {VK_F11, io::Key::F11},
                                           {VK_F12, io::Key::F12},
                                           {VK_F13, io::Key::F13},
                                           {VK_F14, io::Key::F14},
                                           {VK_F15, io::Key::F15},
                                           {VK_F16, io::Key::F16},
                                           {VK_F17, io::Key::F17},
                                           {VK_F18, io::Key::F18},
                                           {VK_F19, io::Key::F19},
                                           {VK_F20, io::Key::F20},
                                           {VK_F21, io::Key::F21},
                                           {VK_F22, io::Key::F22},
                                           {VK_F23, io::Key::F23},
                                           {VK_F24, io::Key::F24},
                                           {VK_NUMPAD0, io::Key::KP0},
                                           {VK_NUMPAD1, io::Key::KP1},
                                           {VK_NUMPAD2, io::Key::KP2},
                                           {VK_NUMPAD3, io::Key::KP3},
                                           {VK_NUMPAD4, io::Key::KP4},
                                           {VK_NUMPAD5, io::Key::KP5},
                                           {VK_NUMPAD6, io::Key::KP6},
                                           {VK_NUMPAD7, io::Key::KP7},
                                           {VK_NUMPAD8, io::Key::KP8},
                                           {VK_NUMPAD9, io::Key::KP9},
                                           {VK_DECIMAL, io::Key::KPDecimal},
                                           {VK_DIVIDE, io::Key::KPDivide},
                                           {VK_MULTIPLY, io::Key::KPMultiply},
                                           {VK_SUBTRACT, io::Key::KPSubtract},
                                           {VK_ADD, io::Key::KPAdd},
                                           {VK_RETURN, io::Key::KPEnter},
                                           {VK_OEM_PLUS, io::Key::KPEqual},
                                           {VK_LSHIFT, io::Key::LeftShift},
                                           {VK_LCONTROL, io::Key::LeftControl},
                                           {VK_LMENU, io::Key::LeftAlt},
                                           {VK_LWIN, io::Key::LeftSuper},
                                           {VK_RSHIFT, io::Key::RightShift},
                                           {VK_RCONTROL, io::Key::RightControl},
                                           {VK_RMENU, io::Key::RightAlt},
                                           {VK_RWIN, io::Key::RightSuper},
                                           {VK_APPS, io::Key::Menu}};
        } ctx;
    } // namespace platform
} // namespace awin

#endif