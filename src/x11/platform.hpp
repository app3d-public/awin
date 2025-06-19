#pragma once

#include <acul/map.hpp>
#include <acul/pair.hpp>
#include <acul/string/string.hpp>
#include <awin/linux/platform.hpp>
#include <cassert>
#include "../linux_cursor_pd.hpp"
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

            struct XIData
            {
                XILoader loader;
                int major_op_code;
                int event_base;
                int error_base;
                int major;
                int minor;
                bool init = false;
            };

            struct XRandrData
            {
                XRandrLoader loader;
                int event_base;
                int error_base;
                int major;
                int minor;
                bool init = false;
                bool gamma_broken;
                bool monitor_broken;
            };

            struct XKBData
            {
                XCBLoader xcb;
                int major_op_code;
                int event_base;
                int error_base;
                int major;
                int minor;
                bool init = false, detectable = false;
                unsigned int group;
            };

            struct XRenderData
            {
                XRenderLoader loader;
                int major;
                int minor;
                int event_base;
                int error_base;
                bool init = false;
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

            struct X11Cursor final : LinuxCursor
            {
                ::Cursor handle = 0;
            };

            extern APPLIB_API struct Context
            {
                X11Loader loader;
                Display *display;
                int screen;
                XID root;
                XID helper_window; // Helper window for IPC
                X11Cursor hidden_cursor;
                XContext context;
                bool utf8 = false;
                XIM im;
                acul::point2D<f32> dpi;
                int empty_pipe[2];
                XIData xi;
                XRandrData xrandr;
                XCursorLoader xcursor;
                XKBData xkb;
                XRenderData xrender;
                int error_code;
                XErrorHandler error_handler = NULL;
                acul::string primary_selection_string, clipboard_string;

                acul::map<i16, io::Key> keymap{{KEY_SPACE, io::Key::Space},
                                               {KEY_APOSTROPHE, io::Key::Apostroph},
                                               {KEY_COMMA, io::Key::Comma},
                                               {KEY_MINUS, io::Key::Minus},
                                               {KEY_PERIOD, io::Key::Period},
                                               {KEY_SLASH, io::Key::Slash},
                                               {KEY_0, io::Key::D0},
                                               {KEY_1, io::Key::D1},
                                               {KEY_2, io::Key::D2},
                                               {KEY_3, io::Key::D3},
                                               {KEY_4, io::Key::D4},
                                               {KEY_5, io::Key::D5},
                                               {KEY_6, io::Key::D6},
                                               {KEY_7, io::Key::D7},
                                               {KEY_8, io::Key::D8},
                                               {KEY_9, io::Key::D9},
                                               {KEY_SEMICOLON, io::Key::Semicolon},
                                               {KEY_EQUAL, io::Key::Equal},
                                               {KEY_A, io::Key::A},
                                               {KEY_B, io::Key::B},
                                               {KEY_C, io::Key::C},
                                               {KEY_D, io::Key::D},
                                               {KEY_E, io::Key::E},
                                               {KEY_F, io::Key::F},
                                               {KEY_G, io::Key::G},
                                               {KEY_H, io::Key::H},
                                               {KEY_I, io::Key::I},
                                               {KEY_J, io::Key::J},
                                               {KEY_K, io::Key::K},
                                               {KEY_L, io::Key::L},
                                               {KEY_M, io::Key::M},
                                               {KEY_N, io::Key::N},
                                               {KEY_O, io::Key::O},
                                               {KEY_P, io::Key::P},
                                               {KEY_Q, io::Key::Q},
                                               {KEY_R, io::Key::R},
                                               {KEY_S, io::Key::S},
                                               {KEY_T, io::Key::T},
                                               {KEY_U, io::Key::U},
                                               {KEY_V, io::Key::V},
                                               {KEY_W, io::Key::W},
                                               {KEY_X, io::Key::X},
                                               {KEY_Y, io::Key::Y},
                                               {KEY_Z, io::Key::Z},
                                               {KEY_BRACKETLEFT, io::Key::LeftBrace},
                                               {KEY_BACKSLASH, io::Key::Backslash},
                                               {KEY_BRACKETRIGHT, io::Key::RightBrace},
                                               {KEY_GRAVE, io::Key::GraveAccent},
                                               {KEY_ESCAPE, io::Key::Escape},
                                               {KEY_RETURN, io::Key::Enter},
                                               {KEY_TAB, io::Key::Tab},
                                               {KEY_BACKSPACE, io::Key::Backspace},
                                               {KEY_INSERT, io::Key::Insert},
                                               {KEY_DELETE, io::Key::Delete},
                                               {KEY_RIGHT, io::Key::Right},
                                               {KEY_LEFT, io::Key::Left},
                                               {KEY_DOWN, io::Key::Down},
                                               {KEY_UP, io::Key::Up},
                                               {KEY_PRIOR, io::Key::PageUp},
                                               {KEY_NEXT, io::Key::PageDown},
                                               {KEY_HOME, io::Key::Home},
                                               {KEY_END, io::Key::End},
                                               {KEY_CAPS_LOCK, io::Key::CapsLock},
                                               {KEY_SCROLL_LOCK, io::Key::ScrollLock},
                                               {KEY_NUM_LOCK, io::Key::NumLock},
                                               {KEY_PRINT, io::Key::PrintScreen},
                                               {KEY_PRINT2, io::Key::PrintScreen},
                                               {KEY_PAUSE, io::Key::Pause},
                                               {KEY_F1, io::Key::F1},
                                               {KEY_F2, io::Key::F2},
                                               {KEY_F3, io::Key::F3},
                                               {KEY_F4, io::Key::F4},
                                               {KEY_F5, io::Key::F5},
                                               {KEY_F6, io::Key::F6},
                                               {KEY_F7, io::Key::F7},
                                               {KEY_F8, io::Key::F8},
                                               {KEY_F9, io::Key::F9},
                                               {KEY_F10, io::Key::F10},
                                               {KEY_F11, io::Key::F11},
                                               {KEY_F12, io::Key::F12},
                                               {KEY_XF86TOOLS2, io::Key::F13},
                                               {KEY_XF86LAUNCH5, io::Key::F14},
                                               {KEY_XF86LAUNCH6, io::Key::F15},
                                               {KEY_XF86LAUNCH7, io::Key::F16},
                                               {KEY_XF86LAUNCH8, io::Key::F17},
                                               {KEY_XF86LAUNCH9, io::Key::F18},
                                               {KEY_F19, io::Key::F19},
                                               {KEY_XF86AUDIOMICMUTE, io::Key::F20},
                                               {KEY_XF86TOUCHPADTOGGLE, io::Key::F21},
                                               {KEY_XF86TOUCHPADON, io::Key::F22},
                                               {KEY_XF86TOUCHPADOFF, io::Key::F23},
                                               {KEY_PROG3, io::Key::F24},
                                               {KEY_KP_INSERT, io::Key::KP0},
                                               {KEY_KP_DELETE, io::Key::KP1},
                                               {KEY_KP_DOWN, io::Key::KP2},
                                               {KEY_KP_NEXT, io::Key::KP3},
                                               {KEY_KP_LEFT, io::Key::KP4},
                                               {KEY_KP_BEGIN, io::Key::KP5},
                                               {KEY_KP_RIGHT, io::Key::KP6},
                                               {KEY_KP_HOME, io::Key::KP7},
                                               {KEY_KP_UP, io::Key::KP8},
                                               {KEY_KP_PRIOR, io::Key::KP9},
                                               {KEY_KP_DECIMAL, io::Key::KPDecimal},
                                               {KEY_KP_DIVIDE, io::Key::KPDivide},
                                               {KEY_KP_MULTIPLY, io::Key::KPMultiply},
                                               {KEY_KP_SUBTRACT, io::Key::KPSubtract},
                                               {KEY_KP_ADD, io::Key::KPAdd},
                                               {KEY_KP_ENTER, io::Key::KPEnter},
                                               {KEY_KP_EQUAL, io::Key::KPEqual},
                                               {KEY_SHIFT_L, io::Key::LeftShift},
                                               {KEY_CONTROL_L, io::Key::LeftControl},
                                               {KEY_ALT_L, io::Key::LeftAlt},
                                               {KEY_SUPER_L, io::Key::LeftSuper},
                                               {KEY_SHIFT_R, io::Key::RightShift},
                                               {KEY_CONTROL_R, io::Key::RightControl},
                                               {KEY_ALT_R, io::Key::RightAlt},
                                               {KEY_SUPER_R, io::Key::RightSuper},
                                               {KEY_MENU, io::Key::Menu}};

                WMAtoms wm;                  // Window manager atoms
                SelectionAtoms select_atoms; // Selection (clipboard) atoms

                ~Context()
                {
                    unload(xkb.xcb.xcb);
                    unload(xrandr.loader.xrandr);
                    unload(xi.loader.xilib);
                    unload(loader.xlib);
                }
            } ctx;

            bool init_platform();

            void destroy_platform();

            // X error handler
            //
            inline int error_handler(Display *display, XErrorEvent *event)
            {
                if (ctx.display != display) return 0;
                ctx.error_code = event->error_code;
                return 0;
            }

            // Sets the X error handler callback
            //
            inline void grab_error_handler()
            {
                assert(ctx.error_handler == NULL);
                ctx.error_code = Success;
                ctx.error_handler = ctx.loader.XSetErrorHandler(error_handler);
            }

            inline void release_error_handler()
            {
                // Synchronize to make sure all commands are processed
                ctx.loader.XSync(ctx.display, False);
                ctx.loader.XSetErrorHandler(ctx.error_handler);
                ctx.error_handler = NULL;
            }

            inline unsigned long get_window_property(XID window, Atom property, Atom type, unsigned char **value)
            {
                Atom actual_type;
                int actual_format;
                unsigned long item_count, bytes_after;

                ctx.loader.XGetWindowProperty(ctx.display, window, property, 0, LONG_MAX, False, type, &actual_type,
                                              &actual_format, &item_count, &bytes_after, value);

                return item_count;
            }

            void init_pcall_data(LinuxPlatformCaller &caller);
            void init_wcall_data(LinuxWindowCaller &caller);

            void init_ccall_data(LinuxCursorCaller &caller);
            platform::native_cursor_t *create_cursor(Cursor::Type);
            void assign_cursor(LinuxWindowImpl *, platform::native_cursor_t *);
            void destroy_cursor(platform::native_cursor_t *);
            bool is_cursor_valid(const platform::native_cursor_t *);
        } // namespace x11
    } // namespace platform
} // namespace awin