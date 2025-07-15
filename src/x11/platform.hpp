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
                acul::map<i16, io::Key> keymap{{X11_KEY_SPACE, io::Key::Space},
                                               {X11_KEY_APOSTROPHE, io::Key::Apostroph},
                                               {X11_KEY_COMMA, io::Key::Comma},
                                               {X11_KEY_MINUS, io::Key::Minus},
                                               {X11_KEY_PERIOD, io::Key::Period},
                                               {X11_KEY_SLASH, io::Key::Slash},
                                               {X11_KEY_0, io::Key::D0},
                                               {X11_KEY_1, io::Key::D1},
                                               {X11_KEY_2, io::Key::D2},
                                               {X11_KEY_3, io::Key::D3},
                                               {X11_KEY_4, io::Key::D4},
                                               {X11_KEY_5, io::Key::D5},
                                               {X11_KEY_6, io::Key::D6},
                                               {X11_KEY_7, io::Key::D7},
                                               {X11_KEY_8, io::Key::D8},
                                               {X11_KEY_9, io::Key::D9},
                                               {X11_KEY_SEMICOLON, io::Key::Semicolon},
                                               {X11_KEY_EQUAL, io::Key::Equal},
                                               {X11_KEY_A, io::Key::A},
                                               {X11_KEY_B, io::Key::B},
                                               {X11_KEY_C, io::Key::C},
                                               {X11_KEY_D, io::Key::D},
                                               {X11_KEY_E, io::Key::E},
                                               {X11_KEY_F, io::Key::F},
                                               {X11_KEY_G, io::Key::G},
                                               {X11_KEY_H, io::Key::H},
                                               {X11_KEY_I, io::Key::I},
                                               {X11_KEY_J, io::Key::J},
                                               {X11_KEY_K, io::Key::K},
                                               {X11_KEY_L, io::Key::L},
                                               {X11_KEY_M, io::Key::M},
                                               {X11_KEY_N, io::Key::N},
                                               {X11_KEY_O, io::Key::O},
                                               {X11_KEY_P, io::Key::P},
                                               {X11_KEY_Q, io::Key::Q},
                                               {X11_KEY_R, io::Key::R},
                                               {X11_KEY_S, io::Key::S},
                                               {X11_KEY_T, io::Key::T},
                                               {X11_KEY_U, io::Key::U},
                                               {X11_KEY_V, io::Key::V},
                                               {X11_KEY_W, io::Key::W},
                                               {X11_KEY_X, io::Key::X},
                                               {X11_KEY_Y, io::Key::Y},
                                               {X11_KEY_Z, io::Key::Z},
                                               {X11_KEY_BRACKETLEFT, io::Key::LeftBrace},
                                               {X11_KEY_BACKSLASH, io::Key::Backslash},
                                               {X11_KEY_BRACKETRIGHT, io::Key::RightBrace},
                                               {X11_KEY_GRAVE, io::Key::GraveAccent},
                                               {X11_KEY_ESCAPE, io::Key::Escape},
                                               {X11_KEY_RETURN, io::Key::Enter},
                                               {X11_KEY_TAB, io::Key::Tab},
                                               {X11_KEY_BACKSPACE, io::Key::Backspace},
                                               {X11_KEY_INSERT, io::Key::Insert},
                                               {X11_KEY_DELETE, io::Key::Delete},
                                               {X11_KEY_RIGHT, io::Key::Right},
                                               {X11_KEY_LEFT, io::Key::Left},
                                               {X11_KEY_DOWN, io::Key::Down},
                                               {X11_KEY_UP, io::Key::Up},
                                               {X11_KEY_PRIOR, io::Key::PageUp},
                                               {X11_KEY_NEXT, io::Key::PageDown},
                                               {X11_KEY_HOME, io::Key::Home},
                                               {X11_KEY_END, io::Key::End},
                                               {X11_KEY_CAPS_LOCK, io::Key::CapsLock},
                                               {X11_KEY_SCROLL_LOCK, io::Key::ScrollLock},
                                               {X11_KEY_NUM_LOCK, io::Key::NumLock},
                                               {X11_KEY_PRINT, io::Key::PrintScreen},
                                               {X11_KEY_PRINT2, io::Key::PrintScreen},
                                               {X11_KEY_PAUSE, io::Key::Pause},
                                               {X11_KEY_F1, io::Key::F1},
                                               {X11_KEY_F2, io::Key::F2},
                                               {X11_KEY_F3, io::Key::F3},
                                               {X11_KEY_F4, io::Key::F4},
                                               {X11_KEY_F5, io::Key::F5},
                                               {X11_KEY_F6, io::Key::F6},
                                               {X11_KEY_F7, io::Key::F7},
                                               {X11_KEY_F8, io::Key::F8},
                                               {X11_KEY_F9, io::Key::F9},
                                               {X11_KEY_F10, io::Key::F10},
                                               {X11_KEY_F11, io::Key::F11},
                                               {X11_KEY_F12, io::Key::F12},
                                               {X11_KEY_XF86TOOLS2, io::Key::F13},
                                               {X11_KEY_XF86LAUNCH5, io::Key::F14},
                                               {X11_KEY_XF86LAUNCH6, io::Key::F15},
                                               {X11_KEY_XF86LAUNCH7, io::Key::F16},
                                               {X11_KEY_XF86LAUNCH8, io::Key::F17},
                                               {X11_KEY_XF86LAUNCH9, io::Key::F18},
                                               {X11_KEY_F19, io::Key::F19},
                                               {X11_KEY_XF86AUDIOMICMUTE, io::Key::F20},
                                               {X11_KEY_XF86TOUCHPADTOGGLE, io::Key::F21},
                                               {X11_KEY_XF86TOUCHPADON, io::Key::F22},
                                               {X11_KEY_XF86TOUCHPADOFF, io::Key::F23},
                                               {X11_KEY_PROG3, io::Key::F24},
                                               {X11_KEY_KP_INSERT, io::Key::KP0},
                                               {X11_KEY_KP_DELETE, io::Key::KP1},
                                               {X11_KEY_KP_DOWN, io::Key::KP2},
                                               {X11_KEY_KP_NEXT, io::Key::KP3},
                                               {X11_KEY_KP_LEFT, io::Key::KP4},
                                               {X11_KEY_KP_BEGIN, io::Key::KP5},
                                               {X11_KEY_KP_RIGHT, io::Key::KP6},
                                               {X11_KEY_KP_HOME, io::Key::KP7},
                                               {X11_KEY_KP_UP, io::Key::KP8},
                                               {X11_KEY_KP_PRIOR, io::Key::KP9},
                                               {X11_KEY_KP_DECIMAL, io::Key::KPDecimal},
                                               {X11_KEY_KP_DIVIDE, io::Key::KPDivide},
                                               {X11_KEY_KP_MULTIPLY, io::Key::KPMultiply},
                                               {X11_KEY_KP_SUBTRACT, io::Key::KPSubtract},
                                               {X11_KEY_KP_ADD, io::Key::KPAdd},
                                               {X11_KEY_KP_ENTER, io::Key::KPEnter},
                                               {X11_KEY_KP_EQUAL, io::Key::KPEqual},
                                               {X11_KEY_SHIFT_L, io::Key::LeftShift},
                                               {X11_KEY_CONTROL_L, io::Key::LeftControl},
                                               {X11_KEY_ALT_L, io::Key::LeftAlt},
                                               {X11_KEY_SUPER_L, io::Key::LeftSuper},
                                               {X11_KEY_SHIFT_R, io::Key::RightShift},
                                               {X11_KEY_CONTROL_R, io::Key::RightControl},
                                               {X11_KEY_ALT_R, io::Key::RightAlt},
                                               {X11_KEY_SUPER_R, io::Key::RightSuper},
                                               {X11_KEY_MENU, io::Key::Menu}};

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