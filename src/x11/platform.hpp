#pragma once

#include <acul/log.hpp>
#include <acul/map.hpp>
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
                acul::map<i16, io::Key> keymap{{X11_KEY_SPACE, io::Key::space},
                                               {X11_KEY_APOSTROPHE, io::Key::apostroph},
                                               {X11_KEY_COMMA, io::Key::comma},
                                               {X11_KEY_MINUS, io::Key::minus},
                                               {X11_KEY_PERIOD, io::Key::period},
                                               {X11_KEY_SLASH, io::Key::slash},
                                               {X11_KEY_0, io::Key::d0},
                                               {X11_KEY_1, io::Key::d1},
                                               {X11_KEY_2, io::Key::d2},
                                               {X11_KEY_3, io::Key::d3},
                                               {X11_KEY_4, io::Key::d4},
                                               {X11_KEY_5, io::Key::d5},
                                               {X11_KEY_6, io::Key::d6},
                                               {X11_KEY_7, io::Key::d7},
                                               {X11_KEY_8, io::Key::d8},
                                               {X11_KEY_9, io::Key::d9},
                                               {X11_KEY_SEMICOLON, io::Key::semicolon},
                                               {X11_KEY_EQUAL, io::Key::equal},
                                               {X11_KEY_A, io::Key::a},
                                               {X11_KEY_B, io::Key::b},
                                               {X11_KEY_C, io::Key::c},
                                               {X11_KEY_D, io::Key::d},
                                               {X11_KEY_E, io::Key::e},
                                               {X11_KEY_F, io::Key::f},
                                               {X11_KEY_G, io::Key::g},
                                               {X11_KEY_H, io::Key::h},
                                               {X11_KEY_I, io::Key::i},
                                               {X11_KEY_J, io::Key::j},
                                               {X11_KEY_K, io::Key::k},
                                               {X11_KEY_L, io::Key::l},
                                               {X11_KEY_M, io::Key::m},
                                               {X11_KEY_N, io::Key::n},
                                               {X11_KEY_O, io::Key::o},
                                               {X11_KEY_P, io::Key::p},
                                               {X11_KEY_Q, io::Key::q},
                                               {X11_KEY_R, io::Key::r},
                                               {X11_KEY_S, io::Key::s},
                                               {X11_KEY_T, io::Key::t},
                                               {X11_KEY_U, io::Key::u},
                                               {X11_KEY_V, io::Key::v},
                                               {X11_KEY_W, io::Key::w},
                                               {X11_KEY_X, io::Key::x},
                                               {X11_KEY_Y, io::Key::y},
                                               {X11_KEY_Z, io::Key::z},
                                               {X11_KEY_BRACKETLEFT, io::Key::lbrace},
                                               {X11_KEY_BACKSLASH, io::Key::backslash},
                                               {X11_KEY_BRACKETRIGHT, io::Key::rbrace},
                                               {X11_KEY_GRAVE, io::Key::grave_accent},
                                               {X11_KEY_ESCAPE, io::Key::escape},
                                               {X11_KEY_RETURN, io::Key::enter},
                                               {X11_KEY_TAB, io::Key::tab},
                                               {X11_KEY_BACKSPACE, io::Key::backspace},
                                               {X11_KEY_INSERT, io::Key::insert},
                                               {X11_KEY_DELETE, io::Key::del},
                                               {X11_KEY_RIGHT, io::Key::right},
                                               {X11_KEY_LEFT, io::Key::left},
                                               {X11_KEY_DOWN, io::Key::down},
                                               {X11_KEY_UP, io::Key::up},
                                               {X11_KEY_PRIOR, io::Key::page_up},
                                               {X11_KEY_NEXT, io::Key::page_down},
                                               {X11_KEY_HOME, io::Key::home},
                                               {X11_KEY_END, io::Key::end},
                                               {X11_KEY_CAPS_LOCK, io::Key::caps_lock},
                                               {X11_KEY_SCROLL_LOCK, io::Key::scroll_lock},
                                               {X11_KEY_NUM_LOCK, io::Key::num_lock},
                                               {X11_KEY_PRINT, io::Key::print_screen},
                                               {X11_KEY_PRINT2, io::Key::print_screen},
                                               {X11_KEY_PAUSE, io::Key::pause},
                                               {X11_KEY_F1, io::Key::f1},
                                               {X11_KEY_F2, io::Key::f2},
                                               {X11_KEY_F3, io::Key::f3},
                                               {X11_KEY_F4, io::Key::f4},
                                               {X11_KEY_F5, io::Key::f5},
                                               {X11_KEY_F6, io::Key::f6},
                                               {X11_KEY_F7, io::Key::f7},
                                               {X11_KEY_F8, io::Key::f8},
                                               {X11_KEY_F9, io::Key::f9},
                                               {X11_KEY_F10, io::Key::f10},
                                               {X11_KEY_F11, io::Key::f11},
                                               {X11_KEY_F12, io::Key::f12},
                                               {X11_KEY_XF86TOOLS2, io::Key::f13},
                                               {X11_KEY_XF86LAUNCH5, io::Key::f14},
                                               {X11_KEY_XF86LAUNCH6, io::Key::f15},
                                               {X11_KEY_XF86LAUNCH7, io::Key::f16},
                                               {X11_KEY_XF86LAUNCH8, io::Key::f17},
                                               {X11_KEY_XF86LAUNCH9, io::Key::f18},
                                               {X11_KEY_F19, io::Key::f19},
                                               {X11_KEY_XF86AUDIOMICMUTE, io::Key::f20},
                                               {X11_KEY_XF86TOUCHPADTOGGLE, io::Key::f21},
                                               {X11_KEY_XF86TOUCHPADON, io::Key::f22},
                                               {X11_KEY_XF86TOUCHPADOFF, io::Key::f23},
                                               {X11_KEY_PROG3, io::Key::f24},
                                               {X11_KEY_KP_INSERT, io::Key::kp_0},
                                               {X11_KEY_KP_DELETE, io::Key::kp_1},
                                               {X11_KEY_KP_DOWN, io::Key::kp_2},
                                               {X11_KEY_KP_NEXT, io::Key::kp_3},
                                               {X11_KEY_KP_LEFT, io::Key::kp_4},
                                               {X11_KEY_KP_BEGIN, io::Key::kp_5},
                                               {X11_KEY_KP_RIGHT, io::Key::kp_6},
                                               {X11_KEY_KP_HOME, io::Key::kp_7},
                                               {X11_KEY_KP_UP, io::Key::kp_8},
                                               {X11_KEY_KP_PRIOR, io::Key::kp_9},
                                               {X11_KEY_KP_DECIMAL, io::Key::kp_decimal},
                                               {X11_KEY_KP_DIVIDE, io::Key::kp_divide},
                                               {X11_KEY_KP_MULTIPLY, io::Key::kp_multiply},
                                               {X11_KEY_KP_SUBTRACT, io::Key::kp_subtract},
                                               {X11_KEY_KP_ADD, io::Key::kp_add},
                                               {X11_KEY_KP_ENTER, io::Key::kp_enter},
                                               {X11_KEY_KP_EQUAL, io::Key::kp_equal},
                                               {X11_KEY_SHIFT_L, io::Key::lshift},
                                               {X11_KEY_CONTROL_L, io::Key::lcontrol},
                                               {X11_KEY_ALT_L, io::Key::lalt},
                                               {X11_KEY_SUPER_L, io::Key::lsuper},
                                               {X11_KEY_SHIFT_R, io::Key::rshift},
                                               {X11_KEY_CONTROL_R, io::Key::rcontrol},
                                               {X11_KEY_ALT_R, io::Key::ralt},
                                               {X11_KEY_SUPER_R, io::Key::rsuper},
                                               {X11_KEY_MENU, io::Key::menu}};

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