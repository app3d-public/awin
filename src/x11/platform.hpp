#pragma once

#include <acul/log.hpp>
#include <acul/lut_table.hpp>
#include <acul/pair.hpp>
#include <acul/string/string.hpp>
#include "../linux_pd.hpp"
#include "keys.hpp"
#include "loaders.hpp"

namespace awin
{
    namespace platform
    {
        namespace x11
        {
            inline void unload(void *&handle)
            {
                if (!handle) return;
                dlclose(handle);
                handle = nullptr;
            }

            struct ExtensionData
            {
                int event_base;
                int error_base;
                int major;
                int minor;
                bool init = false;
            };

            struct XKBData : ExtensionData, XKBLoader
            {
                int major_op_code;
                bool detectable = false;
                unsigned int group;
            };

            struct XCBData : XCBLoader
            {
                bool is_extension_present = false;
            };

            struct XIData : ExtensionData, XILoader
            {
                int major_op_code;
            };

            struct XlibData : XlibLoader
            {
                XKBData xkb;
                XIData xi;
                XCBData xcb;
                XCursorLoader xcursor;
            };

            struct WMAtoms
            {
                Atom NET_SUPPORTED;
                Atom NET_SUPPORTING_WM_CHECK;
                Atom WM_PROTOCOLS;
                Atom WM_STATE;
                Atom WM_DELETE_WINDOW;
                Atom NET_WM_NAME;
                Atom NET_WM_ICON_NAME;
                Atom NET_WM_ICON;
                Atom NET_WM_PID;
                Atom NET_WM_PING;
                Atom NET_WM_WINDOW_TYPE;
                Atom NET_WM_WINDOW_TYPE_NORMAL;
                Atom NET_WM_STATE;
                Atom NET_WM_STATE_ABOVE;
                Atom NET_WM_STATE_FULLSCREEN;
                Atom NET_WM_STATE_MAXIMIZED_VERT;
                Atom NET_WM_STATE_MAXIMIZED_HORZ;
                Atom NET_WM_STATE_DEMANDS_ATTENTION;
                Atom NET_WM_BYPASS_COMPOSITOR;
                Atom NET_WM_FULLSCREEN_MONITORS;
                Atom NET_WM_WINDOW_OPACITY;
                Atom NET_WM_CM_Sx;
                Atom NET_WORKAREA;
                Atom NET_CURRENT_DESKTOP;
                Atom NET_ACTIVE_WINDOW;
                Atom NET_FRAME_EXTENTS;
                Atom NET_REQUEST_FRAME_EXTENTS;
                Atom MOTIF_WM_HINTS;
            };

            struct SelectionAtoms
            {
                Atom TARGETS;
                Atom MULTIPLE;
                Atom INCR;
                Atom CLIPBOARD;
                Atom PRIMARY;
                Atom CLIPBOARD_MANAGER;
                Atom SAVE_TARGETS;
                Atom NULL_;
                Atom UTF8_STRING;
                Atom COMPOUND_STRING;
                Atom ATOM_PAIR;
                Atom WINDOW_SELECTION;
            };

            struct X11Cursor final : Cursor::Platform
            {
                ::Cursor handle = 0;
            };

            struct KeyTraits
            {
                using value_type = uint16_t;
                using enum_type = io::Key;

                static constexpr enum_type unknown = io::Key::unknown;

                static consteval void fill_lut_table(std::array<enum_type, 256> &a)
                {
                    a[X11_KEY_SPACE] = io::Key::space;
                    a[X11_KEY_APOSTROPHE] = io::Key::apostroph;
                    a[X11_KEY_COMMA] = io::Key::comma;
                    a[X11_KEY_MINUS] = io::Key::minus;
                    a[X11_KEY_PERIOD] = io::Key::period;
                    a[X11_KEY_SLASH] = io::Key::slash;
                    a[X11_KEY_0] = io::Key::d0;
                    a[X11_KEY_1] = io::Key::d1;
                    a[X11_KEY_2] = io::Key::d2;
                    a[X11_KEY_3] = io::Key::d3;
                    a[X11_KEY_4] = io::Key::d4;
                    a[X11_KEY_5] = io::Key::d5;
                    a[X11_KEY_6] = io::Key::d6;
                    a[X11_KEY_7] = io::Key::d7;
                    a[X11_KEY_8] = io::Key::d8;
                    a[X11_KEY_9] = io::Key::d9;
                    a[X11_KEY_SEMICOLON] = io::Key::semicolon;
                    a[X11_KEY_EQUAL] = io::Key::equal;
                    a[X11_KEY_A] = io::Key::a;
                    a[X11_KEY_B] = io::Key::b;
                    a[X11_KEY_C] = io::Key::c;
                    a[X11_KEY_D] = io::Key::d;
                    a[X11_KEY_E] = io::Key::e;
                    a[X11_KEY_F] = io::Key::f;
                    a[X11_KEY_G] = io::Key::g;
                    a[X11_KEY_H] = io::Key::h;
                    a[X11_KEY_I] = io::Key::i;
                    a[X11_KEY_J] = io::Key::j;
                    a[X11_KEY_K] = io::Key::k;
                    a[X11_KEY_L] = io::Key::l;
                    a[X11_KEY_M] = io::Key::m;
                    a[X11_KEY_N] = io::Key::n;
                    a[X11_KEY_O] = io::Key::o;
                    a[X11_KEY_P] = io::Key::p;
                    a[X11_KEY_Q] = io::Key::q;
                    a[X11_KEY_R] = io::Key::r;
                    a[X11_KEY_S] = io::Key::s;
                    a[X11_KEY_T] = io::Key::t;
                    a[X11_KEY_U] = io::Key::u;
                    a[X11_KEY_V] = io::Key::v;
                    a[X11_KEY_W] = io::Key::w;
                    a[X11_KEY_X] = io::Key::x;
                    a[X11_KEY_Y] = io::Key::y;
                    a[X11_KEY_Z] = io::Key::z;
                    a[X11_KEY_BRACKETLEFT] = io::Key::lbrace;
                    a[X11_KEY_BACKSLASH] = io::Key::backslash;
                    a[X11_KEY_BRACKETRIGHT] = io::Key::rbrace;
                    a[X11_KEY_GRAVE] = io::Key::grave_accent;
                    a[X11_KEY_ESCAPE] = io::Key::escape;
                    a[X11_KEY_RETURN] = io::Key::enter;
                    a[X11_KEY_TAB] = io::Key::tab;
                    a[X11_KEY_BACKSPACE] = io::Key::backspace;
                    a[X11_KEY_INSERT] = io::Key::insert;
                    a[X11_KEY_DELETE] = io::Key::del;
                    a[X11_KEY_RIGHT] = io::Key::right;
                    a[X11_KEY_LEFT] = io::Key::left;
                    a[X11_KEY_DOWN] = io::Key::down;
                    a[X11_KEY_UP] = io::Key::up;
                    a[X11_KEY_PRIOR] = io::Key::page_up;
                    a[X11_KEY_NEXT] = io::Key::page_down;
                    a[X11_KEY_HOME] = io::Key::home;
                    a[X11_KEY_END] = io::Key::end;
                    a[X11_KEY_CAPS_LOCK] = io::Key::caps_lock;
                    a[X11_KEY_SCROLL_LOCK] = io::Key::scroll_lock;
                    a[X11_KEY_NUM_LOCK] = io::Key::num_lock;
                    a[X11_KEY_PRINT] = io::Key::print_screen;
                    a[X11_KEY_PRINT2] = io::Key::print_screen;
                    a[X11_KEY_PAUSE] = io::Key::pause;
                    a[X11_KEY_F1] = io::Key::f1;
                    a[X11_KEY_F2] = io::Key::f2;
                    a[X11_KEY_F3] = io::Key::f3;
                    a[X11_KEY_F4] = io::Key::f4;
                    a[X11_KEY_F5] = io::Key::f5;
                    a[X11_KEY_F6] = io::Key::f6;
                    a[X11_KEY_F7] = io::Key::f7;
                    a[X11_KEY_F8] = io::Key::f8;
                    a[X11_KEY_F9] = io::Key::f9;
                    a[X11_KEY_F10] = io::Key::f10;
                    a[X11_KEY_F11] = io::Key::f11;
                    a[X11_KEY_F12] = io::Key::f12;
                    a[X11_KEY_XF86TOOLS2] = io::Key::f13;
                    a[X11_KEY_XF86LAUNCH5] = io::Key::f14;
                    a[X11_KEY_XF86LAUNCH6] = io::Key::f15;
                    a[X11_KEY_XF86LAUNCH7] = io::Key::f16;
                    a[X11_KEY_XF86LAUNCH8] = io::Key::f17;
                    a[X11_KEY_XF86LAUNCH9] = io::Key::f18;
                    a[X11_KEY_F19] = io::Key::f19;
                    a[X11_KEY_XF86AUDIOMICMUTE] = io::Key::f20;
                    a[X11_KEY_XF86TOUCHPADTOGGLE] = io::Key::f21;
                    a[X11_KEY_XF86TOUCHPADON] = io::Key::f22;
                    a[X11_KEY_XF86TOUCHPADOFF] = io::Key::f23;
                    a[X11_KEY_PROG3] = io::Key::f24;
                    a[X11_KEY_KP_INSERT] = io::Key::kp_0;
                    a[X11_KEY_KP_DELETE] = io::Key::kp_1;
                    a[X11_KEY_KP_DOWN] = io::Key::kp_2;
                    a[X11_KEY_KP_NEXT] = io::Key::kp_3;
                    a[X11_KEY_KP_LEFT] = io::Key::kp_4;
                    a[X11_KEY_KP_BEGIN] = io::Key::kp_5;
                    a[X11_KEY_KP_RIGHT] = io::Key::kp_6;
                    a[X11_KEY_KP_HOME] = io::Key::kp_7;
                    a[X11_KEY_KP_UP] = io::Key::kp_8;
                    a[X11_KEY_KP_PRIOR] = io::Key::kp_9;
                    a[X11_KEY_KP_DECIMAL] = io::Key::kp_decimal;
                    a[X11_KEY_KP_DIVIDE] = io::Key::kp_divide;
                    a[X11_KEY_KP_MULTIPLY] = io::Key::kp_multiply;
                    a[X11_KEY_KP_SUBTRACT] = io::Key::kp_subtract;
                    a[X11_KEY_KP_ADD] = io::Key::kp_add;
                    a[X11_KEY_KP_ENTER] = io::Key::kp_enter;
                    a[X11_KEY_KP_EQUAL] = io::Key::kp_equal;
                    a[X11_KEY_SHIFT_L] = io::Key::lshift;
                    a[X11_KEY_CONTROL_L] = io::Key::lcontrol;
                    a[X11_KEY_ALT_L] = io::Key::lalt;
                    a[X11_KEY_SUPER_L] = io::Key::lsuper;
                    a[X11_KEY_SHIFT_R] = io::Key::rshift;
                    a[X11_KEY_CONTROL_R] = io::Key::rcontrol;
                    a[X11_KEY_ALT_R] = io::Key::ralt;
                    a[X11_KEY_SUPER_R] = io::Key::rsuper;
                    a[X11_KEY_MENU] = io::Key::menu;
                }
            };

            extern APPLIB_API struct Context
            {
                XlibData xlib;
                Display *display;
                int screen;
                ::Window root;
                ::Window helper_window; // Helper window for IPC
                X11Cursor hidden_cursor;
                XContext context;
                bool utf8 = false;
                XIM im;
                acul::point2D<f32> dpi;
                int empty_pipe[2];
                int error_code;
                XErrorHandler error_handler = NULL;
                acul::string primary_selection_string;
                WindowData *focused_window = nullptr;
                acul::lut_table<256, KeyTraits> keymap;
                WMAtoms wm;                  // Window manager atoms
                SelectionAtoms select_atoms; // Selection (clipboard) atoms

                ~Context()
                {
                    unload(xlib.xcb.handle);
                    unload(xlib.xi.handle);
                    unload(xlib.xcursor.handle);
                    unload(xlib.handle);
                }
            } ctx;

            bool init_platform();

            void destroy_platform();

            // X error handler
            //
            inline int error_handler(Display *display, XErrorEvent *ev)
            {
                if (ctx.display != display) return 0;
                ctx.error_code = ev->error_code;
                char buf[128];
                ctx.xlib.XGetErrorText(ctx.display, ev->error_code, buf, sizeof(buf));
                LOG_ERROR("X11 Error: code=%d (%s), req=%d.%d, res=0x%lx", ev->error_code, buf, ev->request_code,
                          ev->minor_code, ev->resourceid);
                return 0;
            }

            // Sets the X error handler callback
            //
            inline void grab_error_handler()
            {
                assert(ctx.error_handler == NULL);
                ctx.error_code = Success;
                ctx.error_handler = ctx.xlib.XSetErrorHandler(error_handler);
            }

            inline void release_error_handler()
            {
                // Synchronize to make sure all commands are processed
                ctx.xlib.XSync(ctx.display, False);
                ctx.xlib.XSetErrorHandler(ctx.error_handler);
                ctx.error_handler = NULL;
            }

            inline unsigned long get_window_property(XID window, Atom property, Atom type, unsigned char **value)
            {
                Atom actual_type;
                int actual_format;
                unsigned long item_count, bytes_after;

                ctx.xlib.XGetWindowProperty(ctx.display, window, property, 0, LONG_MAX, False, type, &actual_type,
                                            &actual_format, &item_count, &bytes_after, value);

                return item_count;
            }

            void init_pcall_data(LinuxPlatformCaller &caller);
            void init_wcall_data(LinuxWindowCaller &caller);

            void init_ccall_data(LinuxCursorCaller &caller);
            Cursor::Platform *create_cursor(Cursor::Type);
            void assign_cursor(Window *, Cursor::Platform *);
            void destroy_cursor(Cursor::Platform *);
            bool is_cursor_valid(const Cursor::Platform *);
        } // namespace x11
    } // namespace platform
} // namespace awin