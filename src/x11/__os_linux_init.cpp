#include <acul/log.hpp>
#include <awin/window.hpp>
#include <fcntl.h>
#include "../linux_cursor_pd.hpp"
#include "platform.hpp"
#include "window.hpp"

namespace awin
{
    namespace platform
    {
        namespace x11
        {
            Context ctx;

            void set_system_dpi()
            {
                auto &x11 = ctx.loader;
                acul::point2D<f32> dpi{96.0f, 96.0f};

                char *rms = x11.XResourceManagerString(ctx.display);
                if (rms)
                {
                    XrmDatabase db = x11.XrmGetStringDatabase(rms);
                    if (db)
                    {
                        XrmValue value;
                        char *type = NULL;

                        if (x11.XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value))
                        {
                            if (type && strcmp(type, "String") == 0) dpi.x = dpi.y = atof(value.addr);
                        }

                        x11.XrmDestroyDatabase(db);
                    }
                }
                ctx.dpi = dpi / 96.0f;
            }

            bool create_empty_pipe()
            {
                if (pipe(ctx.empty_pipe) != 0)
                {
                    LOG_ERROR("Failed to create empty event pipe: %s", strerror(errno));
                    return false;
                }
                for (int i = 0; i < 2; i++)
                {
                    const int sf = fcntl(ctx.empty_pipe[i], F_GETFL, 0);
                    const int df = fcntl(ctx.empty_pipe[i], F_GETFD, 0);

                    if (sf == -1 || df == -1 || fcntl(ctx.empty_pipe[i], F_SETFL, sf | O_NONBLOCK) == -1 ||
                        fcntl(ctx.empty_pipe[i], F_SETFD, df | FD_CLOEXEC) == -1)
                    {
                        LOG_ERROR("Failed to set flags for empty event pipe: %s", strerror(errno));
                        close(ctx.empty_pipe[i]);
                        return false;
                    }
                }
                return true;
            }

            void init_xi()
            {
                auto &xi = ctx.xi.loader;
                if (!xi.load()) return;
                LOG_INFO("Loaded XInput library");
                if (ctx.loader.XQueryExtension(ctx.display, "XInputExtension", &ctx.xi.major_op_code,
                                               &ctx.xi.event_base, &ctx.xi.error_base) != Success)
                {
                    ctx.xi.major = 2;
                    ctx.xi.minor = 0;
                    ctx.xi.init = xi.XIQueryVersion(ctx.display, &ctx.xi.major, &ctx.xi.minor) == Success;
                }
            }

            void init_xrandr()
            {
                auto &xrandr = ctx.xrandr.loader;
                if (!xrandr.load()) return;
                LOG_INFO("Loaded XRandr library");
                if (xrandr.XRRQueryExtension(ctx.display, &ctx.xrandr.event_base, &ctx.xrandr.error_base))
                {
                    if (xrandr.XRRQueryVersion(ctx.display, &ctx.xrandr.major, &ctx.xrandr.minor))
                        ctx.xrandr.init = ctx.xrandr.major > 1 || ctx.xrandr.minor >= 3;
                    else
                        LOG_ERROR("Failed to query XRandr version");
                }

                if (ctx.xrandr.init)
                {
                    XRRScreenResources *sr = xrandr.XRRGetScreenResourcesCurrent(ctx.display, ctx.root);
                    ctx.xrandr.gamma_broken = !sr->ncrtc || !xrandr.XRRGetCrtcGammaSize(ctx.display, sr->crtcs[0]);
                    ctx.xrandr.monitor_broken = !sr->ncrtc;
                    xrandr.XRRFreeScreenResources(sr);

                    if (!ctx.xrandr.monitor_broken)
                        xrandr.XRRSelectInput(ctx.display, ctx.root, RROutputChangeNotifyMask);
                }
            }

            void init_xkb()
            {
                auto &x11 = ctx.loader;
                ctx.xkb.major = 1;
                ctx.xkb.minor = 0;
                ctx.xkb.init = x11.XkbQueryExtension(ctx.display, &ctx.xkb.major_op_code, &ctx.xkb.event_base,
                                                     &ctx.xkb.error_base, &ctx.xkb.major, &ctx.xkb.minor);
                if (ctx.xkb.init)
                {
                    Bool supported;

                    if (x11.XkbSetDetectableAutoRepeat(ctx.display, True, &supported))
                        if (supported) ctx.xkb.detectable = true;

                    XkbStateRec state;
                    if (x11.XkbGetState(ctx.display, XkbUseCoreKbd, &state) == Success)
                        ctx.xkb.group = (unsigned int)state.group;

                    x11.XkbSelectEventDetails(ctx.display, XkbUseCoreKbd, XkbStateNotify, XkbGroupStateMask,
                                              XkbGroupStateMask);
                }

                if (ctx.xkb.xcb.load()) LOG_INFO("Loaded XCB library");
            }

            // Return the atom ID only if it is listed in the specified array
            //
            Atom get_atom(Atom *supported_atoms, unsigned long atom_count, const char *atom_name)
            {
                const Atom atom = ctx.loader.XInternAtom(ctx.display, atom_name, False);
                for (unsigned long i = 0; i < atom_count; i++)
                    if (supported_atoms[i] == atom) return atom;
                return None;
            }

            static void detect_ewmh()
            {
                auto &x11 = ctx.loader;

                // First we read the _NET_SUPPORTING_WM_CHECK property on the root window
                XID *window_from_root = NULL;
                if (!get_window_property(ctx.root, ctx.wm.NET_SUPPORTING_WM_CHECK, XA_WINDOW,
                                         (unsigned char **)&window_from_root))
                    return;
                grab_error_handler();

                // If it exists, it should be the XID of a top-level window
                // Then we look for the same property on that window

                XID *window_from_child = NULL;
                if (!get_window_property(*window_from_root, ctx.wm.NET_SUPPORTING_WM_CHECK, XA_WINDOW,
                                         (unsigned char **)&window_from_child))
                {
                    release_error_handler();
                    x11.XFree(window_from_root);
                    return;
                }

                release_error_handler();
                // If the property exists, it should contain the XID of the window

                if (*window_from_root != *window_from_child)
                {
                    x11.XFree(window_from_root);
                    x11.XFree(window_from_child);
                    return;
                }

                x11.XFree(window_from_root);
                x11.XFree(window_from_child);

                // We are now fairly sure that an EWMH-compliant WM is currently running
                // We can now start querying the WM about what features it supports by
                // looking in the _NET_SUPPORTED property on the root window
                // It should contain a list of supported EWMH protocol and state atoms

                Atom *supported_atoms = NULL;
                const unsigned long atom_count =
                    get_window_property(ctx.root, ctx.wm.NET_SUPPORTED, XA_ATOM, (unsigned char **)&supported_atoms);

                // See which of the atoms we support that are supported by the WM

                ctx.wm.NET_WM_STATE = get_atom(supported_atoms, atom_count, "_NET_WM_STATE");
                ctx.wm.NET_WM_STATE_ABOVE = get_atom(supported_atoms, atom_count, "_NET_WM_STATE_ABOVE");
                ctx.wm.NET_WM_STATE_FULLSCREEN = get_atom(supported_atoms, atom_count, "_NET_WM_STATE_FULLSCREEN");
                ctx.wm.NET_WM_STATE_MAXIMIZED_VERT =
                    get_atom(supported_atoms, atom_count, "_NET_WM_STATE_MAXIMIZED_VERT");
                ctx.wm.NET_WM_STATE_MAXIMIZED_HORZ =
                    get_atom(supported_atoms, atom_count, "_NET_WM_STATE_MAXIMIZED_HORZ");
                ctx.wm.NET_WM_STATE_DEMANDS_ATTENTION =
                    get_atom(supported_atoms, atom_count, "_NET_WM_STATE_DEMANDS_ATTENTION");
                ctx.wm.NET_WM_FULLSCREEN_MONITORS =
                    get_atom(supported_atoms, atom_count, "_NET_WM_FULLSCREEN_MONITORS");
                ctx.wm.NET_WM_WINDOW_TYPE = get_atom(supported_atoms, atom_count, "_NET_WM_WINDOW_TYPE");
                ctx.wm.NET_WM_WINDOW_TYPE_NORMAL = get_atom(supported_atoms, atom_count, "_NET_WM_WINDOW_TYPE_NORMAL");
                ctx.wm.NET_WORKAREA = get_atom(supported_atoms, atom_count, "_NET_WORKAREA");
                ctx.wm.NET_CURRENT_DESKTOP = get_atom(supported_atoms, atom_count, "_NET_CURRENT_DESKTOP");
                ctx.wm.NET_ACTIVE_WINDOW = get_atom(supported_atoms, atom_count, "_NET_ACTIVE_WINDOW");
                ctx.wm.NET_FRAME_EXTENTS = get_atom(supported_atoms, atom_count, "_NET_FRAME_EXTENTS");
                ctx.wm.NET_REQUEST_FRAME_EXTENTS = get_atom(supported_atoms, atom_count, "_NET_REQUEST_FRAME_EXTENTS");

                if (supported_atoms) x11.XFree(supported_atoms);
            }

            void init_atoms()
            {
                auto &x11 = ctx.loader;

                // String format atoms
                ctx.select_atoms.NULL_ = x11.XInternAtom(ctx.display, "NULL", False);
                ctx.select_atoms.UTF8_STRING = x11.XInternAtom(ctx.display, "UTF8_STRING", False);
                ctx.select_atoms.ATOM_PAIR = x11.XInternAtom(ctx.display, "ATOM_PAIR", False);

                // Custom selection property atom
                ctx.select_atoms.WINDOW_SELECTION = x11.XInternAtom(ctx.display, "WINDOW_SELECTION", False);

                // ICCCM standard clipboard atoms
                ctx.select_atoms.TARGETS = x11.XInternAtom(ctx.display, "TARGETS", False);
                ctx.select_atoms.MULTIPLE = x11.XInternAtom(ctx.display, "MULTIPLE", False);
                ctx.select_atoms.PRIMARY = x11.XInternAtom(ctx.display, "PRIMARY", False);
                ctx.select_atoms.INCR = x11.XInternAtom(ctx.display, "INCR", False);
                ctx.select_atoms.CLIPBOARD = x11.XInternAtom(ctx.display, "CLIPBOARD", False);

                // Clipboard manager atoms
                ctx.select_atoms.CLIPBOARD_MANAGER = x11.XInternAtom(ctx.display, "CLIPBOARD_MANAGER", False);
                ctx.select_atoms.SAVE_TARGETS = x11.XInternAtom(ctx.display, "SAVE_TARGETS", False);

                // ICCCM, EWMH and Motif window property atoms
                // These can be set safely even without WM support
                // The EWMH atoms that require WM support are handled in detectEWMH
                ctx.wm.WM_PROTOCOLS = x11.XInternAtom(ctx.display, "WM_PROTOCOLS", False);
                ctx.wm.WM_STATE = x11.XInternAtom(ctx.display, "WM_STATE", False);
                ctx.wm.WM_DELETE_WINDOW = x11.XInternAtom(ctx.display, "WM_DELETE_WINDOW", False);
                ctx.wm.NET_SUPPORTED = x11.XInternAtom(ctx.display, "_NET_SUPPORTED", False);
                ctx.wm.NET_SUPPORTING_WM_CHECK = x11.XInternAtom(ctx.display, "_NET_SUPPORTING_WM_CHECK", False);
                ctx.wm.NET_WM_ICON = x11.XInternAtom(ctx.display, "_NET_WM_ICON", False);
                ctx.wm.NET_WM_PING = x11.XInternAtom(ctx.display, "_NET_WM_PING", False);
                ctx.wm.NET_WM_PID = x11.XInternAtom(ctx.display, "_NET_WM_PID", False);
                ctx.wm.NET_WM_NAME = x11.XInternAtom(ctx.display, "_NET_WM_NAME", False);
                ctx.wm.NET_WM_ICON_NAME = x11.XInternAtom(ctx.display, "_NET_WM_ICON_NAME", False);
                ctx.wm.NET_WM_BYPASS_COMPOSITOR = x11.XInternAtom(ctx.display, "_NET_WM_BYPASS_COMPOSITOR", False);
                ctx.wm.NET_WM_WINDOW_OPACITY = x11.XInternAtom(ctx.display, "_NET_WM_WINDOW_OPACITY", False);
                ctx.wm.MOTIF_WM_HINTS = x11.XInternAtom(ctx.display, "_MOTIF_WM_HINTS", False);

                // The compositing manager selection name contains the screen number
                {
                    char name[32];
                    snprintf(name, sizeof(name), "_NET_WM_CM_S%u", ctx.screen);
                    ctx.wm.NET_WM_CM_Sx = x11.XInternAtom(ctx.display, name, False);
                }

                // Detect whether an EWMH-conformant window manager is running
                detect_ewmh();
            }

            bool has_usable_input_method_style()
            {
                bool found = false;
                XIMStyles *styles = NULL;
                auto &x11 = ctx.loader;
                if (x11.XGetIMValues(ctx.im, XNQueryInputStyle, &styles, NULL) != NULL) return false;

                for (unsigned int i = 0; i < styles->count_styles; i++)
                {
                    if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing))
                    {
                        found = true;
                        break;
                    }
                }

                x11.XFree(styles);
                return found;
            }

            void input_method_destroy_callback(XIM im, XPointer client_data, XPointer call_data) { ctx.im = NULL; }

            void input_method_instantiate_callback(Display *display, XPointer client_data, XPointer call_data)
            {
                if (ctx.im) return;
                auto &x11 = ctx.loader;

                ctx.im = x11.XOpenIM(ctx.display, 0, NULL, NULL);
                if (ctx.im)
                {
                    if (!has_usable_input_method_style())
                    {
                        x11.XCloseIM(ctx.im);
                        ctx.im = NULL;
                    }
                    else
                    {
                        XIMCallback callback;
                        callback.callback = (XIMProc)input_method_destroy_callback;
                        callback.client_data = NULL;
                        x11.XSetIMValues(ctx.im, XNDestroyCallback, &callback, NULL);

                        XID root, parent;
                        XID *children;
                        unsigned int nchildren;

                        if (x11.XQueryTree(display, ctx.root, &root, &parent, &children, &nchildren))
                        {
                            for (unsigned int i = 0; i < nchildren; i++)
                            {
                                X11WindowData *data = nullptr;
                                if (x11.XFindContext(display, children[i], ctx.context, (XPointer *)&data) == 0 && data)
                                    if (!data->ic) create_input_context(data);
                            }
                            if (children) x11.XFree(children);
                        }
                    }
                }
            }

            // Create a helper window for IPC
            static XID create_helper_window()
            {
                XSetWindowAttributes wa;
                wa.event_mask = PropertyChangeMask;

                return ctx.loader.XCreateWindow(ctx.display, ctx.root, 0, 0, 1, 1, 0, 0, InputOnly,
                                                DefaultVisual(ctx.display, ctx.screen), CWEventMask, &wa);
            }

            X11Cursor create_hidden_cursor()
            {
                X11Cursor result;
                auto &xc = ctx.xcursor;

                XcursorImage *image = xc.XcursorImageCreate(1, 1);
                if (!image) return {};

                image->xhot = 0;
                image->yhot = 0;
                image->pixels[0] = 0; // ARGB = transparent

                result.handle = xc.XcursorImageLoadCursor(ctx.display, image);
                xc.XcursorImageDestroy(image);

                return result;
            }

            bool init_platform()
            {
                auto &x11 = ctx.loader;
                if (!x11.load()) return false;
                if (!x11.XInitThreads || !x11.XrmInitialize || !x11.XOpenDisplay)
                {
                    LOG_ERROR("Failed to load X11 entry points");
                    ctx.loader.unload();
                    return false;
                }
                LOG_INFO("Loaded X11 library");
                x11.XInitThreads();
                x11.XrmInitialize();
                ctx.display = x11.XOpenDisplay(NULL);
                if (!ctx.display)
                {
                    LOG_ERROR("Failed to open X11 display");
                    ctx.loader.unload();
                    return false;
                }
                LOG_INFO("Connected to X11 display");
                ctx.utf8 = x11.Xutf8LookupString && x11.Xutf8SetWMProperties;
                ctx.screen = DefaultScreen(ctx.display);
                ctx.root = RootWindow(ctx.display, ctx.screen);
                ctx.context = (XContext)x11.XrmUniqueQuark();
                set_system_dpi();

                if (!create_empty_pipe()) return false;
                init_xi();
                init_xrandr();
                if (ctx.xcursor.load()) LOG_INFO("Loaded Xcursor library");
                init_xkb();
                auto &xrender = ctx.xrender.loader;
                if (xrender.load() &&
                    xrender.XRenderQueryExtension(ctx.display, &ctx.xrender.event_base, &ctx.xrender.error_base))
                    ctx.xrender.init = xrender.XRenderQueryVersion(ctx.display, &ctx.xrender.major, &ctx.xrender.minor);

                init_atoms();
                ctx.helper_window = create_helper_window();
                ctx.hidden_cursor = create_hidden_cursor();

                if (x11.XSupportsLocale() && ctx.utf8)
                {
                    x11.XSetLocaleModifiers("");
                    // If an IM is already present our callback will be called right away
                    x11.XRegisterIMInstantiateCallback(ctx.display, NULL, NULL, NULL, input_method_instantiate_callback,
                                                       NULL);
                }

                LOG_INFO("Created X11 Window Context");
                return true;
            }

            APPLIB_API void destroy_platform()
            {
                auto &x11 = ctx.loader;
                if (ctx.helper_window)
                {
                    if (x11.XGetSelectionOwner(ctx.display, ctx.select_atoms.CLIPBOARD) == ctx.helper_window)
                        push_selection_to_manager_x11();

                    x11.XDestroyWindow(ctx.display, ctx.helper_window);
                    ctx.helper_window = None;
                }

                x11.XUnregisterIMInstantiateCallback(ctx.display, NULL, NULL, NULL, input_method_instantiate_callback,
                                                     NULL);
                if (ctx.im)
                {
                    x11.XCloseIM(ctx.im);
                    ctx.im = NULL;
                }

                if (ctx.display)
                {
                    x11.XCloseDisplay(ctx.display);
                    ctx.display = NULL;
                }

                if (ctx.empty_pipe[0] || ctx.empty_pipe[1])
                {
                    close(ctx.empty_pipe[0]);
                    close(ctx.empty_pipe[1]);
                }
            }

            LinuxWindowImpl *alloc_window_impl() { return acul::alloc<X11WindowData>(); }

            void init_pcall_data(LinuxPlatformCaller &caller)
            {
                caller.init_platform = init_platform;
                caller.destroy_platform = destroy_platform;
                caller.alloc_window_impl = alloc_window_impl;
                caller.poll_events = poll_events;
                caller.wait_events = wait_events;
                caller.wait_events_timeout = wait_events_timeout;
            }

            void init_wcall_data(LinuxWindowCaller &caller)
            {
                caller.destroy = destroy;
                caller.create_window = create_window;
                caller.set_window_icon = set_window_icon;
                caller.show_window = show_window;
                caller.hide_window = hide_window;
                caller.get_window_title = get_window_title;
                caller.set_window_title = set_window_title;
                caller.enable_fullscreen = enable_fullscreen;
                caller.disable_fullscreen = disable_fullscreen;
                caller.get_cursor_position = get_cursor_position;
                caller.set_cursor_position = set_cursor_position;
                caller.hide_cursor = hide_cursor;
                caller.show_cursor = show_cursor;
                caller.get_window_position = get_window_position;
                caller.set_window_position = set_window_position;
            }

            void init_ccall_data(LinuxCursorCaller &caller)
            {
                caller.create = create_cursor;
                caller.assign = assign_cursor;
                caller.destroy = destroy_cursor;
                caller.valid = is_cursor_valid;
            }
        } // namespace x11
    } // namespace platform
} // namespace awin