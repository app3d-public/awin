#pragma once

#include <acul/lut_table.hpp>
#include <acul/pair.hpp>
#include <awin/types.hpp>
#include <windows.h>

namespace awin
{
    namespace platform
    {
        struct KeyTraits
        {
            using value_type = uint16_t;
            using enum_type = io::Key;

            static constexpr enum_type unknown = io::Key::unknown;

            static consteval void fill_lut_table(std::array<enum_type, 256> &a)
            {
                a[VK_SPACE] = io::Key::space;
                a[VK_OEM_7] = io::Key::apostroph;
                a[VK_OEM_COMMA] = io::Key::comma;
                a[VK_OEM_MINUS] = io::Key::minus;
                a[VK_OEM_PERIOD] = io::Key::period;
                a[VK_OEM_2] = io::Key::slash;
                a[0x30] = io::Key::d0;
                a[0x31] = io::Key::d1;
                a[0x32] = io::Key::d2;
                a[0x33] = io::Key::d3;
                a[0x34] = io::Key::d4;
                a[0x35] = io::Key::d5;
                a[0x36] = io::Key::d6;
                a[0x37] = io::Key::d7;
                a[0x38] = io::Key::d8;
                a[0x39] = io::Key::d9;
                a[VK_OEM_1] = io::Key::semicolon;
                a[VK_OEM_PLUS] = io::Key::equal;
                a[0x41] = io::Key::a;
                a[0x42] = io::Key::b;
                a[0x43] = io::Key::c;
                a[0x44] = io::Key::d;
                a[0x45] = io::Key::e;
                a[0x46] = io::Key::f;
                a[0x47] = io::Key::g;
                a[0x48] = io::Key::h;
                a[0x49] = io::Key::i;
                a[0x4A] = io::Key::j;
                a[0x4B] = io::Key::k;
                a[0x4C] = io::Key::l;
                a[0x4D] = io::Key::m;
                a[0x4E] = io::Key::n;
                a[0x4F] = io::Key::o;
                a[0x50] = io::Key::p;
                a[0x51] = io::Key::q;
                a[0x52] = io::Key::r;
                a[0x53] = io::Key::s;
                a[0x54] = io::Key::t;
                a[0x55] = io::Key::u;
                a[0x56] = io::Key::v;
                a[0x57] = io::Key::w;
                a[0x58] = io::Key::x;
                a[0x59] = io::Key::y;
                a[0x5A] = io::Key::z;
                a[VK_OEM_4] = io::Key::lbrace;
                a[VK_OEM_5] = io::Key::backslash;
                a[VK_OEM_6] = io::Key::rbrace;
                a[VK_OEM_3] = io::Key::grave_accent;
                a[VK_ESCAPE] = io::Key::escape;
                a[VK_RETURN] = io::Key::enter;
                a[VK_TAB] = io::Key::tab;
                a[VK_BACK] = io::Key::backspace;
                a[VK_INSERT] = io::Key::insert;
                a[VK_DELETE] = io::Key::del;
                a[VK_RIGHT] = io::Key::right;
                a[VK_LEFT] = io::Key::left;
                a[VK_DOWN] = io::Key::down;
                a[VK_UP] = io::Key::up;
                a[VK_PRIOR] = io::Key::page_up;
                a[VK_NEXT] = io::Key::page_down;
                a[VK_HOME] = io::Key::home;
                a[VK_END] = io::Key::end;
                a[VK_CAPITAL] = io::Key::caps_lock;
                a[VK_SCROLL] = io::Key::scroll_lock;
                a[VK_NUMLOCK] = io::Key::num_lock;
                a[VK_SNAPSHOT] = io::Key::print_screen;
                a[VK_PAUSE] = io::Key::pause;
                a[VK_F1] = io::Key::f1;
                a[VK_F2] = io::Key::f2;
                a[VK_F3] = io::Key::f3;
                a[VK_F4] = io::Key::f4;
                a[VK_F5] = io::Key::f5;
                a[VK_F6] = io::Key::f6;
                a[VK_F7] = io::Key::f7;
                a[VK_F8] = io::Key::f8;
                a[VK_F9] = io::Key::f9;
                a[VK_F10] = io::Key::f10;
                a[VK_F11] = io::Key::f11;
                a[VK_F12] = io::Key::f12;
                a[VK_F13] = io::Key::f13;
                a[VK_F14] = io::Key::f14;
                a[VK_F15] = io::Key::f15;
                a[VK_F16] = io::Key::f16;
                a[VK_F17] = io::Key::f17;
                a[VK_F18] = io::Key::f18;
                a[VK_F19] = io::Key::f19;
                a[VK_F20] = io::Key::f20;
                a[VK_F21] = io::Key::f21;
                a[VK_F22] = io::Key::f22;
                a[VK_F23] = io::Key::f23;
                a[VK_F24] = io::Key::f24;
                a[VK_NUMPAD0] = io::Key::kp_0;
                a[VK_NUMPAD1] = io::Key::kp_1;
                a[VK_NUMPAD2] = io::Key::kp_2;
                a[VK_NUMPAD3] = io::Key::kp_3;
                a[VK_NUMPAD4] = io::Key::kp_4;
                a[VK_NUMPAD5] = io::Key::kp_5;
                a[VK_NUMPAD6] = io::Key::kp_6;
                a[VK_NUMPAD7] = io::Key::kp_7;
                a[VK_NUMPAD8] = io::Key::kp_8;
                a[VK_NUMPAD9] = io::Key::kp_9;
                a[VK_DECIMAL] = io::Key::kp_decimal;
                a[VK_DIVIDE] = io::Key::kp_divide;
                a[VK_MULTIPLY] = io::Key::kp_multiply;
                a[VK_SUBTRACT] = io::Key::kp_subtract;
                a[VK_ADD] = io::Key::kp_add;
                a[VK_RETURN] = io::Key::kp_enter;
                a[VK_OEM_PLUS] = io::Key::kp_equal;
                a[VK_LSHIFT] = io::Key::lshift;
                a[VK_LCONTROL] = io::Key::lcontrol;
                a[VK_LMENU] = io::Key::lalt;
                a[VK_LWIN] = io::Key::lsuper;
                a[VK_RSHIFT] = io::Key::rshift;
                a[VK_RCONTROL] = io::Key::rcontrol;
                a[VK_RMENU] = io::Key::ralt;
                a[VK_RWIN] = io::Key::rsuper;
                a[VK_APPS] = io::Key::menu;
            }
        };

        extern struct Context
        {
            HINSTANCE instance;
            WNDCLASSEXW win32_class;
            DWORD thread_id;
            int padding;
            acul::point2D<int> frame, screen;
            UINT dpi;
            acul::lut_table<256, KeyTraits> keymap;
        } ctx;

    } // namespace platform
} // namespace awin