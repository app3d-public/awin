#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/cursorfont.h>
#include <acul/log.hpp>
#include <awin/window.hpp>
#include "acul/pair.hpp"
#include "awin/linux/platform.hpp"
#include "awin/platform.hpp"
#include "awin/types.hpp"
#include "platform.hpp"
#include "window.hpp"

// MWM_HINTS constants
#define MWM_HINTS_FUNCTIONS   (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_FUNC_ALL          (1L << 0)
#define MWM_FUNC_RESIZE       (1L << 1)
#define MWM_FUNC_MOVE         (1L << 2)
#define MWM_FUNC_MINIMIZE     (1L << 3)
#define MWM_FUNC_MAXIMIZE     (1L << 4)
#define MWM_FUNC_CLOSE        (1L << 5)
#define MWM_DECOR_ALL         0xFFFFFFFF

namespace awin
{
    namespace platform
    {
        namespace x11
        {

            void input_context_destroy_callback(XIC ic, XPointer clientData, XPointer callData)
            {
                X11WindowData *window = (X11WindowData *)clientData;
                window->ic = NULL;
            }

            void create_input_context(X11WindowData *window)
            {
                XIMCallback callback;
                auto &x11 = ctx.loader;
                callback.callback = (XIMProc)input_context_destroy_callback;
                callback.client_data = (XPointer)window;

                window->ic =
                    x11.XCreateIC(ctx.im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow,
                                  window->window, XNFocusWindow, window->window, XNDestroyCallback, &callback, NULL);

                if (window->ic)
                {
                    XWindowAttributes attribs;
                    x11.XGetWindowAttributes(ctx.display, window->window, &attribs);

                    unsigned long filter = 0;
                    if (x11.XGetICValues(window->ic, XNFilterEvents, &filter, NULL) == NULL)
                        x11.XSelectInput(ctx.display, window->window, attribs.your_event_mask | filter);
                }
            }

            // Returns whether the event is a selection event
            static Bool is_selection_event(Display *display, XEvent *event, XPointer pointer)
            {
                if (event->xany.window != ctx.helper_window) return False;
                return event->type == SelectionRequest || event->type == SelectionNotify ||
                       event->type == SelectionClear;
            }

            // Set the specified property to the selection converted to the requested target
            static Atom write_target_to_property(const XSelectionRequestEvent *request)
            {
                auto &x11 = ctx.loader;
                char *selection_string = NULL;
                const Atom formats[] = {ctx.select_atoms.UTF8_STRING, XA_STRING};
                const int format_count = sizeof(formats) / sizeof(formats[0]);

                if (request->selection == ctx.select_atoms.PRIMARY)
                    selection_string = (char *)ctx.primary_selection_string.c_str();
                else
                    selection_string = (char *)ctx.clipboard_string.c_str();

                if (request->property == None)
                {
                    // The requester is a legacy client (ICCCM section 2.2)
                    // We don't support legacy clients, so fail here
                    return None;
                }

                if (request->target == ctx.select_atoms.TARGETS)
                {
                    // The list of supported targets was requested

                    const Atom targets[] = {ctx.select_atoms.TARGETS, ctx.select_atoms.MULTIPLE,
                                            ctx.select_atoms.UTF8_STRING, XA_STRING};

                    x11.XChangeProperty(ctx.display, request->requestor, request->property, XA_ATOM, 32,
                                        PropModeReplace, (unsigned char *)targets,
                                        sizeof(targets) / sizeof(targets[0]));

                    return request->property;
                }

                if (request->target == ctx.select_atoms.MULTIPLE)
                {
                    // Multiple conversions were requested
                    Atom *targets;
                    const unsigned long count = get_window_property(
                        request->requestor, request->property, ctx.select_atoms.ATOM_PAIR, (unsigned char **)&targets);

                    for (unsigned long i = 0; i < count; i += 2)
                    {
                        int j;
                        for (j = 0; j < format_count; j++)
                            if (targets[i] == formats[j]) break;

                        if (j < format_count)
                            x11.XChangeProperty(ctx.display, request->requestor, targets[i + 1], targets[i], 8,
                                                PropModeReplace, (unsigned char *)selection_string,
                                                null_terminated_length(selection_string));
                        else
                            targets[i + 1] = None;
                    }

                    x11.XChangeProperty(ctx.display, request->requestor, request->property, ctx.select_atoms.ATOM_PAIR,
                                        32, PropModeReplace, (unsigned char *)targets, count);
                    x11.XFree(targets);

                    return request->property;
                }

                if (request->target == ctx.select_atoms.SAVE_TARGETS)
                {
                    // The request is a check whether we support SAVE_TARGETS
                    // It should be handled as a no-op side effect target

                    x11.XChangeProperty(ctx.display, request->requestor, request->property, ctx.select_atoms.NULL_, 32,
                                        PropModeReplace, NULL, 0);
                    return request->property;
                }

                // Conversion to a data target was requested

                for (int i = 0; i < format_count; i++)
                {
                    if (request->target == formats[i])
                    {
                        // The requested target is one we support

                        x11.XChangeProperty(ctx.display, request->requestor, request->property, request->target, 8,
                                            PropModeReplace, (unsigned char *)selection_string,
                                            null_terminated_length(selection_string));

                        return request->property;
                    }
                }

                // The requested target is not supported
                return None;
            }

            static void handle_selection_request(XEvent *event)
            {
                const XSelectionRequestEvent *request = &event->xselectionrequest;

                XEvent reply = {SelectionNotify};
                reply.xselection.property = write_target_to_property(request);
                reply.xselection.display = request->display;
                reply.xselection.requestor = request->requestor;
                reply.xselection.selection = request->selection;
                reply.xselection.target = request->target;
                reply.xselection.time = request->time;

                ctx.loader.XSendEvent(ctx.display, request->requestor, False, 0, &reply);
            }

            void push_selection_to_manager_x11()
            {
                auto &x11 = ctx.loader;
                x11.XConvertSelection(ctx.display, ctx.select_atoms.CLIPBOARD_MANAGER, ctx.select_atoms.SAVE_TARGETS,
                                      None, ctx.helper_window, CurrentTime);

                for (;;)
                {
                    XEvent event;

                    while (x11.XCheckIfEvent(ctx.display, &event, is_selection_event, NULL))
                    {
                        switch (event.type)
                        {
                            case SelectionRequest:
                                handle_selection_request(&event);
                                break;

                            case SelectionNotify:
                            {
                                if (event.xselection.target == ctx.select_atoms.SAVE_TARGETS)
                                {
                                    // This means one of two things; either the selection
                                    // was not owned, which means there is no clipboard
                                    // manager, or the transfer to the clipboard manager has
                                    // completed
                                    // In either case, it means we are done here
                                    return;
                                }

                                break;
                            }
                        }
                    }

                    wait_for_x11_event(NULL);
                }
            }

            // Applies MWM hints: decorations and functions
            static void apply_motif_hints(Display *dpy, XID window, WindowFlags window_flags)
            {
                struct MotifWmHints
                {
                    unsigned long flags;
                    unsigned long functions;
                    unsigned long decorations;
                    long input_mode;
                    unsigned long status;
                } hints = {0};

                hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
                // functions
                hints.functions = MWM_FUNC_MOVE | MWM_FUNC_CLOSE;
                if (window_flags & WindowFlagBits::Resizable) hints.functions |= MWM_FUNC_RESIZE;
                if (window_flags & WindowFlagBits::MinimizeBox) hints.functions |= MWM_FUNC_MINIMIZE;
                if (window_flags & WindowFlagBits::MaximizeBox) hints.functions |= MWM_FUNC_MAXIMIZE;
                // decorations
                hints.decorations = window_flags & WindowFlagBits::Decorated ? MWM_DECOR_ALL : 0;
                auto &x11 = ctx.loader;
                x11.XChangeProperty(dpy, window, ctx.wm.MOTIF_WM_HINTS, ctx.wm.MOTIF_WM_HINTS, 32, PropModeReplace,
                                    reinterpret_cast<unsigned char *>(&hints), sizeof(hints) / sizeof(long));
            }

            acul::string get_window_title(platform::LinuxWindowImpl *window_data)
            {
                auto *x11_data = (X11WindowData *)window_data;
                auto &x11 = ctx.loader;
                Atom actual_type;
                int actual_format;
                unsigned long nitems, bytes_after;
                unsigned char *prop = nullptr;

                int status = x11.XGetWindowProperty(ctx.display, x11_data->window, ctx.wm.NET_WM_NAME, 0, (~0L), False,
                                                    ctx.select_atoms.UTF8_STRING, &actual_type, &actual_format, &nitems,
                                                    &bytes_after, &prop);

                if (status == Success && prop && actual_type == ctx.select_atoms.UTF8_STRING)
                {
                    acul::string result(reinterpret_cast<const char *>(prop), nitems);
                    x11.XFree(prop);
                    return result;
                }
                return {};
            }

            void set_window_title(platform::LinuxWindowImpl *window_data, const acul::string &title)
            {
                auto *x11_data = (X11WindowData *)window_data;
                auto &x11 = ctx.loader;
                const char *pTitle = title.c_str();
                if (ctx.utf8)
                    x11.Xutf8SetWMProperties(ctx.display, x11_data->window, pTitle, pTitle, NULL, 0, NULL, NULL, NULL);
                x11.XChangeProperty(ctx.display, x11_data->window, ctx.wm.NET_WM_NAME, ctx.select_atoms.UTF8_STRING, 8,
                                    PropModeReplace, (unsigned char *)pTitle, title.length());
                x11.XChangeProperty(ctx.display, x11_data->window, ctx.wm.NET_WM_ICON_NAME,
                                    ctx.select_atoms.UTF8_STRING, 8, PropModeReplace, (unsigned char *)pTitle,
                                    title.length());
                x11.XFlush(ctx.display);
            }

            void enable_fullscreen(LinuxWindowImpl *window_data)
            {
                auto *x11 = (X11WindowData *)window_data;
                if (!ctx.wm.NET_WM_STATE || !ctx.wm.NET_WM_STATE_FULLSCREEN) return;

                XEvent e = {};
                e.xclient.type = ClientMessage;
                e.xclient.window = x11->window;
                e.xclient.message_type = ctx.wm.NET_WM_STATE;
                e.xclient.format = 32;
                e.xclient.data.l[0] = 1;
                e.xclient.data.l[1] = ctx.wm.NET_WM_STATE_FULLSCREEN;
                e.xclient.data.l[2] = 0;
                e.xclient.data.l[3] = 1;
                e.xclient.data.l[4] = 0;

                ctx.loader.XSendEvent(ctx.display, ctx.root, False, SubstructureNotifyMask | SubstructureRedirectMask,
                                      &e);
                ctx.loader.XFlush(ctx.display);
            }

            void disable_fullscreen(LinuxWindowImpl *window_data)
            {
                auto *x11 = (X11WindowData *)window_data;
                if (!ctx.wm.NET_WM_STATE || !ctx.wm.NET_WM_STATE_FULLSCREEN) return;

                XEvent e = {};
                e.xclient.type = ClientMessage;
                e.xclient.window = x11->window;
                e.xclient.message_type = ctx.wm.NET_WM_STATE;
                e.xclient.format = 32;
                e.xclient.data.l[0] = 0; // _NET_WM_STATE_REMOVE
                e.xclient.data.l[1] = ctx.wm.NET_WM_STATE_FULLSCREEN;
                e.xclient.data.l[2] = 0;
                e.xclient.data.l[3] = 1;
                e.xclient.data.l[4] = 0;

                ctx.loader.XSendEvent(ctx.display, ctx.root, False, SubstructureNotifyMask | SubstructureRedirectMask,
                                      &e);
                ctx.loader.XFlush(ctx.display);
            }

            bool bind_wm_to_window(X11WindowData *window, const acul::string &title, i32 width, i32 height,
                                   WindowFlags flags)
            {
                auto &x11 = ctx.loader;
                if (ctx.wm.NET_WM_STATE)
                {
                    Atom states[3];
                    int count = 0;

                    if (flags & WindowFlagBits::Maximized && ctx.wm.NET_WM_STATE_MAXIMIZED_VERT &&
                        ctx.wm.NET_WM_STATE_MAXIMIZED_HORZ)
                    {
                        states[count++] = ctx.wm.NET_WM_STATE_MAXIMIZED_VERT;
                        states[count++] = ctx.wm.NET_WM_STATE_MAXIMIZED_HORZ;
                    }

                    if (count)
                        x11.XChangeProperty(ctx.display, window->window, ctx.wm.NET_WM_STATE, XA_ATOM, 32,
                                            PropModeReplace, (unsigned char *)states, count);
                }

                // Declare the WM protocols supported by Window System
                {
                    Atom protocols[] = {ctx.wm.WM_DELETE_WINDOW, ctx.wm.NET_WM_PING};
                    x11.XSetWMProtocols(ctx.display, window->window, protocols, sizeof(protocols) / sizeof(Atom));
                }

                // Declare our PID
                {
                    const long pid = getpid();
                    x11.XChangeProperty(ctx.display, window->window, ctx.wm.NET_WM_PID, XA_CARDINAL, 32,
                                        PropModeReplace, (unsigned char *)&pid, 1);
                }

                if (ctx.wm.NET_WM_WINDOW_TYPE && ctx.wm.NET_WM_WINDOW_TYPE_NORMAL)
                {
                    Atom type = ctx.wm.NET_WM_WINDOW_TYPE_NORMAL;
                    x11.XChangeProperty(ctx.display, window->window, ctx.wm.NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                                        PropModeReplace, (unsigned char *)&type, 1);
                }

                // Set ICCCM WM_HINTS property
                {
                    XWMHints *hints = x11.XAllocWMHints();
                    if (!hints)
                    {
                        LOG_ERROR("Failed to allocate WM hints");
                        return false;
                    }

                    hints->flags = StateHint;
                    hints->initial_state = NormalState;

                    x11.XSetWMHints(ctx.display, window->window, hints);
                    x11.XFree(hints);
                }

                // Set ICCCM WM_NORMAL_HINTS property
                {
                    XSizeHints *hints = x11.XAllocSizeHints();
                    if (!hints)
                    {
                        LOG_ERROR("Failed to allocate size hints");
                        return false;
                    }

                    if (!(flags & WindowFlagBits::Resizable))
                    {
                        hints->flags |= (PMinSize | PMaxSize);
                        hints->min_width = hints->max_width = width;
                        hints->min_height = hints->max_height = height;
                    }

                    hints->flags |= PPosition;
                    hints->x = 0;
                    hints->y = 0;

                    hints->flags |= PWinGravity;
                    hints->win_gravity = StaticGravity;

                    x11.XSetWMNormalHints(ctx.display, window->window, hints);
                    x11.XFree(hints);
                }

                // Set ICCCM WM_CLASS property
                {
                    XClassHint *hint = x11.XAllocClassHint();

                    const char *resource_name = getenv("RESOURCE_NAME");
                    if (resource_name && strlen(resource_name))
                        hint->res_name = (char *)resource_name;
                    else if (!title.empty())
                        hint->res_name = (char *)title.c_str();
                    else
                        hint->res_name = (char *)"window-application";

                    if (!title.empty())
                        hint->res_class = (char *)title.c_str();
                    else
                        hint->res_class = (char *)"Window-Application";

                    x11.XSetClassHint(ctx.display, window->window, hint);
                    x11.XFree(hint);
                }
                return true;
            }

            bool wait_for_x11_event(f64 *timeout)
            {
                struct pollfd fd = {ConnectionNumber(ctx.display), POLLIN};
                while (!ctx.loader.XPending(ctx.display))
                    if (!poll_posix(&fd, 1, timeout)) return false;
                return true;
            }

            // Wait for event data to arrive on any event file descriptor
            // This avoids blocking other threads via the per-display Xlib lock that also
            // covers GLX functions
            //
            static bool wait_for_any_event(f64 *timeout)
            {
                struct pollfd fds[] = {
                    {ConnectionNumber(ctx.display), POLLIN}, {ctx.empty_pipe[0], POLLIN}, {-1, POLLIN}};

                while (!ctx.loader.XPending(ctx.display))
                {
                    if (!poll_posix(fds, sizeof(fds) / sizeof(fds[0]), timeout)) return false;
                    for (int i = 1; i < sizeof(fds) / sizeof(fds[0]); i++)
                        if (fds[i].revents & POLLIN) return true;
                }

                return true;
            }

            inline bool wait_for_visibility_notify(X11WindowData *window_data)
            {
                XEvent dummy;
                f64 timeout = 0.1;
                while (!ctx.loader.XCheckTypedWindowEvent(ctx.display, window_data->window, VisibilityNotify, &dummy))
                    if (!wait_for_x11_event(&timeout)) return false;
                return true;
            }

            void show_window(platform::LinuxWindowImpl *window_data)
            {
                auto *x11 = (X11WindowData *)window_data;
                ctx.loader.XMapWindow(ctx.display, x11->window);
                wait_for_visibility_notify(x11);
            }

            void hide_window(platform::LinuxWindowImpl *window_data)
            {
                auto *x11 = (X11WindowData *)window_data;
                ctx.loader.XUnmapWindow(ctx.display, x11->window);
                ctx.loader.XFlush(ctx.display);
            }

            // Writes a byte to the empty event pipe
            static void write_empty_event()
            {
                while (true)
                {
                    const char byte = 0;
                    const ssize_t result = write(ctx.empty_pipe[1], &byte, 1);
                    if (result == 1 || (result == -1 && errno != EINTR)) break;
                }
            }

            // Drains available data from the empty event pipe
            static void drain_empty_events()
            {
                while (true)
                {
                    char dummy[64];
                    const ssize_t result = read(ctx.empty_pipe[0], dummy, sizeof(dummy));
                    if (result == -1 && errno != EINTR) break;
                }
            }

            // Returns whether the window is iconified
            static int get_window_state(X11WindowData *window)
            {
                int result = WithdrawnState;
                struct
                {
                    CARD32 state;
                    Window icon;
                } *state = NULL;

                if (get_window_property(window->window, ctx.wm.WM_STATE, ctx.wm.WM_STATE, (unsigned char **)&state) >=
                    2)
                    result = state->state;

                if (state) ctx.loader.XFree(state);

                return result;
            }

            // Returns whether it is a _NET_FRAME_EXTENTS event for the specified window
            static Bool is_frame_extents_event(Display *display, XEvent *event, XPointer pointer)
            {
                X11WindowData *window = (X11WindowData *)pointer;
                return event->type == PropertyNotify && event->xproperty.state == PropertyNewValue &&
                       event->xproperty.window == window->window && event->xproperty.atom == ctx.wm.NET_FRAME_EXTENTS;
            }

            // Sends an EWMH or ICCCM event to the window manager
            static void send_event_to_wm(X11WindowData *window_data, Atom type, long a, long b, long c, long d, long e)
            {
                XEvent event = {ClientMessage};
                event.xclient.window = window_data->window;
                event.xclient.format = 32;
                event.xclient.message_type = type;
                event.xclient.data.l[0] = a;
                event.xclient.data.l[1] = b;
                event.xclient.data.l[2] = c;
                event.xclient.data.l[3] = d;
                event.xclient.data.l[4] = e;
                auto &x11 = ctx.loader;
                x11.XSendEvent(ctx.display, ctx.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &event);
            }

            // Grabs the cursor and confines it to the window
            static void capture_cursor(X11WindowData *window_data)
            {
                ctx.loader.XGrabPointer(ctx.display, window_data->window, True,
                                        ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
                                        GrabModeAsync, window_data->window, None, CurrentTime);
            }

            // Ungrabs the cursor
            static void release_cursor() { ctx.loader.XUngrabPointer(ctx.display, CurrentTime); }

            acul::point2D<i32> get_cursor_position(platform::LinuxWindowImpl *window_data)
            {
                auto *x11_data = (X11WindowData *)window_data;
                auto &x11 = ctx.loader;
                XID root_return, child_return;
                int root_x, root_y, win_x, win_y;
                unsigned int mask_return;

                if (x11.XQueryPointer(ctx.display, x11_data->window, &root_return, &child_return, &root_x, &root_y,
                                      &win_x, &win_y, &mask_return))
                    return {win_x, win_y};
                return {};
            }

            void set_cursor_position(platform::LinuxWindowImpl *window_data, acul::point2D<i32> position)
            {
                acul::point2D<int> abs_pos;
                auto &x11 = ctx.loader;
                auto *xd = reinterpret_cast<X11WindowData *>(window_data);
                x11.XTranslateCoordinates(ctx.display, xd->window, ctx.root, position.x, position.y, &abs_pos.x,
                                          &abs_pos.y, &ctx.helper_window);
                x11.XWarpPointer(ctx.display, ctx.root, ctx.root, 0, 0, 0, 0, abs_pos.x, abs_pos.y);
                x11.XFlush(ctx.display);
            }

            // From __os_linux_keys.cpp
            void on_key_press(XEvent *, int, Bool, platform::WindowData *);
            void on_key_release(XEvent *, int, platform::WindowData *);
            void on_btn_press(XEvent *, platform::WindowData *);
            void on_btn_release(XEvent *, platform::WindowData *);

            void on_client_msg(XEvent *event, platform::WindowData *window_data)
            {
                auto &x11 = ctx.loader;
                if (event->xclient.message_type == ctx.wm.WM_PROTOCOLS)
                {
                    const Atom protocol = event->xclient.data.l[0];
                    if (protocol == None) return;
                    if (protocol == ctx.wm.WM_DELETE_WINDOW)
                        window_data->ready_to_close = true;
                    else if (protocol == ctx.wm.NET_WM_PING)
                    {
                        // The window manager is pinging the application to ensure
                        // it's still responding to events
                        XEvent reply = *event;
                        reply.xclient.window = ctx.root;
                        x11.XSendEvent(ctx.display, ctx.root, False, SubstructureNotifyMask | SubstructureRedirectMask,
                                       &reply);
                    }
                }
                // todo: make DnD here
            }

            bool is_window_maximized(X11WindowData *window_data)
            {
                Atom *states;
                bool maximized = false;

                if (!ctx.wm.NET_WM_STATE || !ctx.wm.NET_WM_STATE_MAXIMIZED_VERT || !ctx.wm.NET_WM_STATE_MAXIMIZED_HORZ)
                    return maximized;

                const unsigned long count =
                    get_window_property(window_data->window, ctx.wm.NET_WM_STATE, XA_ATOM, (unsigned char **)&states);

                for (unsigned long i = 0; i < count; i++)
                {
                    if (states[i] == ctx.wm.NET_WM_STATE_MAXIMIZED_VERT ||
                        states[i] == ctx.wm.NET_WM_STATE_MAXIMIZED_HORZ)
                    {
                        maximized = true;
                        break;
                    }
                }

                if (states) ctx.loader.XFree(states);
                return maximized;
            }

            // Process the specified X event
            static void process_event(XEvent *event)
            {
                auto &x11 = ctx.loader;
                int keycode = 0;
                Bool filtered = False;

                // HACK: Save scancode as some IMs clear the field in XFilterEvent
                if (event->type == KeyPress || event->type == KeyRelease) keycode = event->xkey.keycode;
                filtered = x11.XFilterEvent(event, None);

                if (ctx.xkb.init)
                {
                    if (event->type == ctx.xkb.event_base + XkbEventCode)
                    {
                        if (((XkbEvent *)event)->any.xkb_type == XkbStateNotify &&
                            (((XkbEvent *)event)->state.changed & XkbGroupStateMask))
                            ctx.xkb.group = ((XkbEvent *)event)->state.group;
                        return;
                    }
                }

                platform::WindowData *window_data = NULL;
                if (x11.XFindContext(ctx.display, event->xany.window, ctx.context, (XPointer *)&window_data) != 0)
                    return;

                if (event->type == GenericEvent && ctx.xi.init && event->xcookie.extension == ctx.xi.major_op_code &&
                    x11.XGetEventData(ctx.display, &event->xcookie) && event->xcookie.evtype == XI_RawMotion)
                {
                    if (window_data->backend.raw_input)
                    {
                        XIRawEvent *raw = (XIRawEvent *)event->xcookie.data;
                        acul::point2D<i32> delta{0, 0};
                        int idx = 0;
                        if (XIMaskIsSet(raw->valuators.mask, 0)) delta.x = raw->raw_values[idx++];
                        if (XIMaskIsSet(raw->valuators.mask, 1)) delta.y = raw->raw_values[idx++];
                        dispatch_window_event(event_registry.mouse_move, event_id::MouseMove, window_data->owner,
                                              delta);
                    }
                    x11.XFreeEventData(ctx.display, &event->xcookie);
                    return;
                }

                if (event->type == SelectionRequest)
                {
                    handle_selection_request(event);
                    return;
                }
                auto *window = (X11WindowData *)window_data->backend.impl;

                switch (event->type)
                {
                    case ReparentNotify:
                        window->parent = event->xreparent.parent;
                        return;
                    case KeyPress:
                        on_key_press(event, keycode, filtered, window_data);
                        return;
                    case KeyRelease:
                        on_key_release(event, keycode, window_data);
                        return;
                    case ButtonPress:
                        on_btn_press(event, window_data);
                        return;
                    case ButtonRelease:
                        on_btn_release(event, window_data);
                        return;
                    case EnterNotify:
                    {
                        dispatch_window_event(event_registry.mouse_enter, window_data->owner, true);
                        acul::point2D dim{event->xcrossing.x, event->xcrossing.y};
                        dispatch_window_event(event_registry.mouse_move_abs, event_id::MouseMoveAbs, window_data->owner,
                                              dim);
                        return;
                    }
                    case LeaveNotify:
                    {
                        dispatch_window_event(event_registry.mouse_enter, window_data->owner, false);
                        return;
                    }
                    case MotionNotify:
                    {
                        acul::point2D pos{event->xmotion.x, event->xmotion.y};
                        dispatch_window_event(event_registry.mouse_move_abs, event_id::MouseMoveAbs, window_data->owner,
                                              pos);
                        return;
                    }
                    case ConfigureNotify:
                    {
                        acul::point2D<i32> dimenstions(event->xconfigure.width, event->xconfigure.height);
                        if (dimenstions != window_data->dimenstions)
                        {
                            window_data->dimenstions = dimenstions;
                            dispatch_window_event(event_registry.resize, event_id::Resize, window_data->owner,
                                                  window_data->dimenstions);
                        }
                        acul::point2D<i32> pos(event->xconfigure.x, event->xconfigure.y);

                        // NOTE: ConfigureNotify events from the server are in local
                        //       coordinates, so if we are reparented we need to translate
                        //       the position into root (screen) coordinates
                        if (!event->xany.send_event && window->parent != ctx.root)
                        {
                            grab_error_handler();

                            XID dummy;
                            x11.XTranslateCoordinates(ctx.display, window->parent, ctx.root, pos.x, pos.y, &pos.x,
                                                      &pos.y, &dummy);

                            release_error_handler();
                            if (ctx.error_code == BadWindow) return;
                        }

                        if (window_data->backend.pos != pos)
                        {
                            window_data->backend.pos = pos;
                            dispatch_window_event(event_registry.move, event_id::Move, window_data->owner, pos);
                        }
                        return;
                    }
                    case ClientMessage:
                        if (!filtered && event->xclient.message_type != None) on_client_msg(event, window_data);
                        return;

                    case FocusIn:
                    {
                        if (event->xfocus.mode == NotifyGrab || event->xfocus.mode == NotifyUngrab)
                        {
                            // Ignore focus events from popup indicator windows, window menu
                            // key chords and window dragging
                            return;
                        }
                        if (window_data->is_cursor_hidden) capture_cursor(window);
                        if (window->ic) x11.XSetICFocus(window->ic);

                        dispatch_window_event(event_registry.focus, window_data->owner, true);
                        return;
                    }

                    case FocusOut:
                    {
                        if (event->xfocus.mode == NotifyGrab || event->xfocus.mode == NotifyUngrab)
                        {
                            // Ignore focus events from popup indicator windows, window menu
                            // key chords and window dragging
                            return;
                        }
                        if (window_data->is_cursor_hidden) release_cursor();
                        if (window->ic) x11.XUnsetICFocus(window->ic);

                        dispatch_window_event(event_registry.focus, window_data->owner, false);
                        return;
                    }
                    case PropertyNotify:
                    {
                        if (event->xproperty.state != PropertyNewValue) return;

                        if (event->xproperty.atom == ctx.wm.WM_STATE)
                        {
                            const int state = get_window_state(window);
                            if (state != IconicState && state != NormalState) return;

                            const bool iconified = (state == IconicState);
                            const bool already = (window_data->flags & WindowFlagBits::Minimized);

                            if (iconified != already)
                            {
                                if (iconified)
                                    window_data->flags |= WindowFlagBits::Minimized;
                                else
                                    window_data->flags &= ~WindowFlagBits::Minimized;

                                dispatch_window_event(event_registry.minimize, event_id::Minimize, window_data->owner,
                                                      iconified);
                            }
                        }
                        else if (event->xproperty.atom == ctx.wm.NET_WM_STATE)
                        {
                            const bool maximized = is_window_maximized(window);
                            const bool already = (window_data->flags & WindowFlagBits::Maximized);

                            if (maximized != already)
                            {
                                if (maximized)
                                    window_data->flags |= WindowFlagBits::Maximized;
                                else
                                    window_data->flags &= ~WindowFlagBits::Maximized;

                                dispatch_window_event(event_registry.maximize, event_id::Maximize, window_data->owner,
                                                      maximized);
                            }
                        }

                        return;
                    }
                    default:
                        return;
                }
            }

            bool prepare_window_wm_hints(X11WindowData *x11_data, WindowFlags flags, acul::point2D<i32> dim,
                                         const acul::string &title)
            {
                auto &x11 = ctx.loader;

                // ICCCM WM_HINTS: initial state (Normal or Iconic)
                {
                    XWMHints *hints = x11.XAllocWMHints();
                    hints->flags = StateHint;
                    hints->initial_state = (flags & WindowFlagBits::Minimized) ? IconicState : NormalState;
                    x11.XSetWMHints(ctx.display, x11_data->window, hints);
                    x11.XFree(hints);
                }

                // Declare the WM protocols supported by Awin
                {
                    Atom protocols[] = {ctx.wm.WM_DELETE_WINDOW, ctx.wm.NET_WM_PING};
                    x11.XSetWMProtocols(ctx.display, x11_data->window, protocols, sizeof(protocols) / sizeof(Atom));
                }

                // Declare our PID
                {
                    const long pid = getpid();
                    x11.XChangeProperty(ctx.display, x11_data->window, ctx.wm.NET_WM_PID, XA_CARDINAL, 32,
                                        PropModeReplace, (unsigned char *)&pid, 1);
                }

                if (ctx.wm.NET_WM_WINDOW_TYPE && ctx.wm.NET_WM_WINDOW_TYPE_NORMAL)
                {
                    Atom type = ctx.wm.NET_WM_WINDOW_TYPE_NORMAL;
                    x11.XChangeProperty(ctx.display, x11_data->window, ctx.wm.NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                                        PropModeReplace, (unsigned char *)&type, 1);
                }

                // Set ICCCM WM_HINTS property
                {
                    XWMHints *hints = x11.XAllocWMHints();
                    if (!hints)
                    {
                        LOG_ERROR("Failed to allocate WM hints");
                        return false;
                    }

                    hints->flags = StateHint;
                    hints->initial_state = NormalState;

                    x11.XSetWMHints(ctx.display, x11_data->window, hints);
                    x11.XFree(hints);
                }

                // Set ICCCM WM_NORMAL_HINTS property
                {
                    XSizeHints *hints = x11.XAllocSizeHints();
                    if (!hints)
                    {
                        LOG_ERROR("Failed to allocate size hints");
                        return false;
                    }

                    if (!(flags & WindowFlagBits::Resizable))
                    {
                        hints->flags |= (PMinSize | PMaxSize);
                        hints->min_width = hints->max_width = dim.x;
                        hints->min_height = hints->max_height = dim.y;
                    }

                    hints->flags |= PWinGravity;
                    hints->win_gravity = StaticGravity;

                    x11.XSetWMNormalHints(ctx.display, x11_data->window, hints);
                    x11.XFree(hints);
                }

                // Set ICCCM WM_CLASS property
                {
                    XClassHint *hint = x11.XAllocClassHint();
                    const char *resource_name = getenv("RESOURCE_NAME");
                    bool is_title_valid = !title.empty();
                    if (resource_name && null_terminated_length(resource_name))
                        hint->res_name = (char *)resource_name;
                    else
                        hint->res_name = is_title_valid ? (char *)title.c_str() : (char *)"awin-application";
                    hint->res_class = is_title_valid ? (char *)title.c_str() : (char *)"awin-Application";
                    x11.XSetClassHint(ctx.display, x11_data->window, hint);
                    x11.XFree(hint);
                }
                return true;
            }

            inline void get_window_pos(const X11WindowData *window_data, acul::point2D<i32> &pos)
            {
                XID dummy;
                ctx.loader.XTranslateCoordinates(ctx.display, window_data->window, ctx.root, 0, 0, &pos.x, &pos.y,
                                                 &dummy);
            }

            inline void get_window_size(const X11WindowData *window_data, acul::point2D<i32> &size)
            {
                XWindowAttributes attribs;
                ctx.loader.XGetWindowAttributes(ctx.display, window_data->window, &attribs);
            }

            bool create_window(platform::WindowData *window_data, const acul::string &title, i32 width, i32 height,
                               WindowFlags flags)
            {
                auto &x11 = ctx.loader;
                Visual *visual = DefaultVisual(ctx.display, ctx.screen);
                int depth = DefaultDepth(ctx.display, ctx.screen);
                // Create a colormap based on the visual used by the current context
                auto *x11_data = (X11WindowData *)window_data->backend.impl;
                x11_data->colormap = x11.XCreateColormap(ctx.display, ctx.root, visual, AllocNone);

                XSetWindowAttributes wa = {0};
                wa.colormap = x11_data->colormap;
                wa.background_pixel = BlackPixel(ctx.display, ctx.screen);
                wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask |
                                ButtonPressMask | ButtonReleaseMask | ExposureMask | FocusChangeMask |
                                VisibilityChangeMask | EnterWindowMask | LeaveWindowMask | PropertyChangeMask;
                grab_error_handler();
                x11_data->parent = ctx.root;
                x11_data->window =
                    x11.XCreateWindow(ctx.display, ctx.root, 0, 0, width, height,
                                      0,     // Border width
                                      depth, // Color depth
                                      InputOutput, visual, CWBorderPixel | CWBackPixel | CWColormap | CWEventMask, &wa);
                release_error_handler();
                if (!x11_data->window)
                {
                    LOG_ERROR("Failed to create window");
                    return false;
                }
                LOG_INFO("Created X11 window: %lu", x11_data->window);
                x11.XSaveContext(ctx.display, x11_data->window, ctx.context, (XPointer)window_data);
                window_data->flags = flags;
                apply_motif_hints(ctx.display, x11_data->window, flags);

                // EWMH: fullscreen or maximized
                if (ctx.wm.NET_WM_STATE)
                {
                    acul::vector<Atom> wm_states;
                    if ((flags & WindowFlagBits::Fullscreen))
                    {
                        if (ctx.wm.NET_WM_STATE_FULLSCREEN) wm_states.push_back(ctx.wm.NET_WM_STATE_FULLSCREEN);
                        if (ctx.wm.NET_WM_BYPASS_COMPOSITOR)
                        {
                            unsigned long one = 1;
                            x11.XChangeProperty(ctx.display, x11_data->window, ctx.wm.NET_WM_BYPASS_COMPOSITOR,
                                                XA_CARDINAL, 32, PropModeReplace,
                                                reinterpret_cast<unsigned char *>(&one), 1);
                        }
                    }
                    if ((flags & WindowFlagBits::Maximized) && ctx.wm.NET_WM_STATE_MAXIMIZED_VERT &&
                        ctx.wm.NET_WM_STATE_MAXIMIZED_HORZ)
                    {
                        wm_states.push_back(ctx.wm.NET_WM_STATE_MAXIMIZED_VERT);
                        wm_states.push_back(ctx.wm.NET_WM_STATE_MAXIMIZED_HORZ);
                    }
                    if (!wm_states.empty())
                        x11.XChangeProperty(ctx.display, x11_data->window, ctx.wm.NET_WM_STATE, XA_ATOM, 32,
                                            PropModeReplace, reinterpret_cast<unsigned char *>(wm_states.data()),
                                            static_cast<int>(wm_states.size()));
                }

                if (!prepare_window_wm_hints(x11_data, flags, {width, height}, title)) return false;
                if (ctx.im) create_input_context(x11_data);

                set_window_title(x11_data, title);
                get_window_pos(x11_data, window_data->backend.pos);
                get_window_size(x11_data, window_data->dimenstions);

                if (!(flags & WindowFlagBits::Hidden)) show_window(x11_data);
                return true;
            }

            void destroy(LinuxWindowImpl *impl)
            {
                auto *x11_data = (X11WindowData *)impl;
                auto &x11 = ctx.loader;
                if (x11_data->ic)
                {
                    x11.XDestroyIC(x11_data->ic);
                    x11_data->ic = NULL;
                }

                if (x11_data->window)
                {
                    LOG_INFO("Destroying Window: %lu", x11_data->window);
                    x11.XDeleteContext(ctx.display, x11_data->window, ctx.context);
                    x11.XUnmapWindow(ctx.display, x11_data->window);
                    x11.XDestroyWindow(ctx.display, x11_data->window);
                    x11_data->window = (XID)0;
                }

                if (x11_data->colormap)
                {
                    x11.XFreeColormap(ctx.display, x11_data->colormap);
                    x11_data->colormap = (Colormap)0;
                }

                x11.XFlush(ctx.display);
            }

            acul::point2D<i32> get_window_position(LinuxWindowImpl *window)
            {
                auto *x11_data = (X11WindowData *)window;
                ::Window dummy;
                acul::point2D<i32> r;
                ctx.loader.XTranslateCoordinates(ctx.display, x11_data->window, ctx.root, 0, 0, &r.x, &r.y, &dummy);
                return r;
            }

            void set_window_position(WindowData *window, acul::point2D<i32> position)
            {
                auto *x11_data = (X11WindowData *)window->backend.impl;
                auto &x11 = ctx.loader;
                if (window->flags & WindowFlagBits::Hidden)
                {
                    long supplied;
                    XSizeHints *hints = x11.XAllocSizeHints();

                    if (x11.XGetWMNormalHints(ctx.display, x11_data->window, hints, &supplied))
                    {
                        hints->flags |= PPosition;
                        hints->x = hints->y = 0;

                        x11.XSetWMNormalHints(ctx.display, x11_data->window, hints);
                    }

                    x11.XFree(hints);
                }
                x11.XMoveWindow(ctx.display, x11_data->window, position.x, position.y);
                x11.XFlush(ctx.display);
            }

            void poll_events()
            {
                drain_empty_events();
                auto &x11 = ctx.loader;

                x11.XPending(ctx.display);

                while (QLength(ctx.display))
                {
                    XEvent event;
                    x11.XNextEvent(ctx.display, &event);
                    process_event(&event);
                }

                x11.XFlush(ctx.display);
            }

            void wait_events()
            {
                wait_for_any_event(NULL);
                poll_events();
            }

            void wait_events_timeout()
            {
                wait_for_any_event(&env.timeout);
                poll_events();
            }

            void set_window_icon(LinuxWindowImpl *w, const acul::vector<Image> &images)
            {
                size_t total = 0;
                for (const auto &img : images) { total += 2 + size_t(img.dimenstions.x) * size_t(img.dimenstions.y); }

                acul::vector<unsigned long> buf;
                buf.reserve(total);

                for (const auto &img : images)
                {
                    buf.push_back(static_cast<unsigned long>(img.dimenstions.x));
                    buf.push_back(static_cast<unsigned long>(img.dimenstions.y));

                    // RGBA8 to ARGB32 (CARDINAL)
                    const uint8_t *p = reinterpret_cast<const uint8_t *>(img.pixels);
                    int pixels_count = img.dimenstions.x * img.dimenstions.y;
                    for (int i = 0; i < pixels_count; ++i)
                    {
                        u32 r = p[i * 4 + 0];
                        u32 g = p[i * 4 + 1];
                        u32 b = p[i * 4 + 2];
                        u32 a = p[i * 4 + 3];
                        unsigned long argb =
                            (static_cast<unsigned long>(a) << 24) | (static_cast<unsigned long>(r) << 16) |
                            (static_cast<unsigned long>(g) << 8) | (static_cast<unsigned long>(b) << 0);
                        buf.push_back(argb);
                    }
                }

                auto &x11 = ctx.loader;
                auto *window = reinterpret_cast<X11WindowData *>(w);
                x11.XChangeProperty(ctx.display, window->window, ctx.wm.NET_WM_ICON, XA_CARDINAL, 32, PropModeReplace,
                                    reinterpret_cast<const unsigned char *>(buf.data()), static_cast<int>(buf.size()));
                x11.XFlush(ctx.display);
            }

            ::Cursor load_system_cursor(Display *display, const char *name, unsigned int fallback_shape)
            {
                if (ctx.xcursor.XcursorLibraryLoadCursor)
                {
                    if (auto cur = ctx.xcursor.XcursorLibraryLoadCursor(display, name)) return cur;
                }

                return ctx.loader.XCreateFontCursor(display, fallback_shape);
            }

            LinuxCursor *create_cursor(Cursor::Type type)
            {
                struct Shape
                {
                    const char *name;
                    unsigned int fallback;
                };

                static const acul::hashmap<Cursor::Type, Shape> cursor_map = {
                    {Cursor::Type::Arrow, {"left_ptr", XC_left_ptr}},
                    {Cursor::Type::Ibeam, {"xterm", XC_xterm}},
                    {Cursor::Type::Crosshair, {"crosshair", XC_crosshair}},
                    {Cursor::Type::Hand, {"hand2", XC_hand2}},
                    {Cursor::Type::ResizeEW, {"sb_h_double_arrow", XC_sb_h_double_arrow}},
                    {Cursor::Type::ResizeNS, {"sb_v_double_arrow", XC_sb_v_double_arrow}},
                    {Cursor::Type::ResizeNWSE, {"bottom_right_corner", XC_bottom_right_corner}},
                    {Cursor::Type::ResizeNESW, {"bottom_left_corner", XC_bottom_left_corner}},
                    {Cursor::Type::ResizeAll, {"fleur", XC_fleur}},
                    {Cursor::Type::NotAllowed, {"not-allowed", XC_pirate}}, // fallback shape
                };

                auto it = cursor_map.find(type);
                if (it == cursor_map.end()) return nullptr;

                const auto &entry = it->second;

                auto *cursor = acul::alloc<X11Cursor>();
                cursor->handle = load_system_cursor(ctx.display, entry.name, entry.fallback);
                return cursor;
            }

            void assign_cursor(LinuxWindowImpl *window_data, LinuxCursor *cursor)
            {
                auto *x11_cursor = (X11Cursor *)cursor;
                auto &x11 = ctx.loader;
                auto *x11_data = (X11WindowData *)window_data;
                if (x11_cursor->valid())
                    x11.XDefineCursor(ctx.display, x11_data->window, x11_cursor->handle);
                else
                    x11.XUndefineCursor(ctx.display, x11_data->window);
                x11.XFlush(ctx.display);
            }

            void destroy_cursor(LinuxCursor *cursor)
            {
                auto *x11_cursor = (X11Cursor *)cursor;
                if (x11_cursor->handle)
                {
                    ctx.loader.XFreeCursor(ctx.display, x11_cursor->handle);
                    x11_cursor->handle = 0;
                }
            }

            bool is_cursor_valid(const platform::native_cursor_t *cursor)
            {
                auto *x11_cursor = (X11Cursor *)cursor;
                return x11_cursor->handle != 0;
            }

            void hide_cursor(WindowData *window_data)
            {
                if (window_data->is_cursor_hidden) return;
                auto *x11_data = (X11WindowData *)window_data->backend.impl;
                ctx.loader.XDefineCursor(ctx.display, x11_data->window, ctx.hidden_cursor.handle);
                ctx.loader.XFlush(ctx.display);
                window_data->is_cursor_hidden = true;
            }

            void show_cursor(WindowData *window_data)
            {
                if (!window_data->is_cursor_hidden) return;
                auto *x11_data = (X11WindowData *)window_data->backend.impl;
                ctx.loader.XUndefineCursor(ctx.display, x11_data->window);
                ctx.loader.XFlush(ctx.display);
                window_data->is_cursor_hidden = false;
            }
        } // namespace x11
    } // namespace platform
} // namespace awin