#pragma once

#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/shape.h>
#include <X11/keysym.h>
#include <dlfcn.h>
#include <xcb/xcb.h>

typedef XClassHint *(*PFN_XAllocClassHint)(void);
typedef XSizeHints *(*PFN_XAllocSizeHints)(void);
typedef XWMHints *(*PFN_XAllocWMHints)(void);
typedef int (*PFN_XChangeProperty)(Display *, XID, Atom, Atom, int, int, const unsigned char *, int);
typedef int (*PFN_XChangeWindowAttributes)(Display *, XID, unsigned long, XSetWindowAttributes *);
typedef Bool (*PFN_XCheckIfEvent)(Display *, XEvent *, Bool (*)(Display *, XEvent *, XPointer), XPointer);
typedef Bool (*PFN_XCheckTypedWindowEvent)(Display *, XID, int, XEvent *);
typedef int (*PFN_XCloseDisplay)(Display *);
typedef Status (*PFN_XCloseIM)(XIM);
typedef int (*PFN_XConvertSelection)(Display *, Atom, Atom, Atom, XID, Time);
typedef Colormap (*PFN_XCreateColormap)(Display *, XID, Visual *, int);
typedef ::Cursor (*PFN_XCreateFontCursor)(Display *, unsigned int);
typedef XIC (*PFN_XCreateIC)(XIM, ...);
typedef Region (*PFN_XCreateRegion)(void);
typedef XID (*PFN_XCreateWindow)(Display *, XID, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int,
                                 Visual *, unsigned long, XSetWindowAttributes *);
typedef int (*PFN_XDefineCursor)(Display *, XID, ::Cursor);
typedef int (*PFN_XDeleteContext)(Display *, XID, XContext);
typedef int (*PFN_XDeleteProperty)(Display *, XID, Atom);
typedef void (*PFN_XDestroyIC)(XIC);
typedef int (*PFN_XDestroyRegion)(Region);
typedef int (*PFN_XDestroyWindow)(Display *, XID);
typedef int (*PFN_XDisplayKeycodes)(Display *, int *, int *);
typedef int (*PFN_XEventsQueued)(Display *, int);
typedef Bool (*PFN_XFilterEvent)(XEvent *, XID);
typedef int (*PFN_XFindContext)(Display *, XID, XContext, XPointer *);
typedef int (*PFN_XFlush)(Display *);
typedef int (*PFN_XFree)(void *);
typedef char *(*PFN_XGetAtomName)(Display *, Atom);
typedef int (*PFN_XFreeColormap)(Display *, Colormap);
typedef int (*PFN_XFreeCursor)(Display *, ::Cursor);
typedef void (*PFN_XFreeEventData)(Display *, XGenericEventCookie *);
typedef int (*PFN_XGetErrorText)(Display *, int, char *, int);
typedef Bool (*PFN_XGetEventData)(Display *, XGenericEventCookie *);
typedef char *(*PFN_XGetICValues)(XIC, ...);
typedef char *(*PFN_XGetIMValues)(XIM, ...);
typedef int (*PFN_XGetInputFocus)(Display *, XID *, int *);
typedef KeySym *(*PFN_XGetKeyboardMapping)(Display *, KeyCode, int, int *);
typedef int (*PFN_XGetScreenSaver)(Display *, int *, int *, int *, int *);
typedef XID (*PFN_XGetSelectionOwner)(Display *, Atom);
typedef XVisualInfo *(*PFN_XGetVisualInfo)(Display *, long, XVisualInfo *, int *);
typedef Status (*PFN_XGetWMNormalHints)(Display *, XID, XSizeHints *, long *);
typedef Status (*PFN_XGetWindowAttributes)(Display *, XID, XWindowAttributes *);
typedef int (*PFN_XGetWindowProperty)(Display *, XID, Atom, long, long, Bool, Atom, Atom *, int *, unsigned long *,
                                      unsigned long *, unsigned char **);
typedef int (*PFN_XGrabPointer)(Display *, XID, Bool, unsigned int, int, int, XID, ::Cursor, Time);
typedef Status (*PFN_XIconifyWindow)(Display *, XID, int);
typedef Status (*PFN_XInitThreads)(void);
typedef Atom (*PFN_XInternAtom)(Display *, const char *, Bool);
typedef int (*PFN_XLookupString)(XKeyEvent *, char *, int, KeySym *, XComposeStatus *);
typedef int (*PFN_Xutf8LookupString)(XIC, XKeyPressedEvent *, char *, int, KeySym *, Status *);
typedef void (*PFN_Xutf8SetWMProperties)(Display *, XID, const char *, const char *, char **, int, XSizeHints *,
                                         XWMHints *, XClassHint *);
typedef int (*PFN_XMapRaised)(Display *, XID);
typedef int (*PFN_XMapWindow)(Display *, XID);
typedef int (*PFN_XMoveResizeWindow)(Display *, XID, int, int, unsigned int, unsigned int);
typedef int (*PFN_XMoveWindow)(Display *, XID, int, int);
typedef int (*PFN_XNextEvent)(Display *, XEvent *);
typedef Display *(*PFN_XOpenDisplay)(const char *);
typedef XIM (*PFN_XOpenIM)(Display *, XrmDatabase *, char *, char *);
typedef int (*PFN_XPeekEvent)(Display *, XEvent *);
typedef int (*PFN_XPending)(Display *);
typedef void (*PFN_XrmDestroyDatabase)(XrmDatabase);
typedef Bool (*PFN_XrmGetResource)(XrmDatabase, const char *, const char *, char **, XrmValue *);
typedef XrmDatabase (*PFN_XrmGetStringDatabase)(const char *);
typedef void (*PFN_XrmInitialize)(void);
typedef XrmQuark (*PFN_XrmUniqueQuark)(void);
typedef Bool (*PFN_XQueryExtension)(Display *, const char *, int *, int *, int *);
typedef Bool (*PFN_XQueryPointer)(Display *, XID, XID *, XID *, int *, int *, int *, int *, unsigned int *);
typedef Status (*PFN_XQueryTree)(Display *, XID, XID *, XID *, XID **, unsigned int *);
typedef int (*PFN_XRaiseWindow)(Display *, XID);
typedef Bool (*PFN_XRegisterIMInstantiateCallback)(Display *, void *, char *, char *, XIDProc, XPointer);
typedef Bool (*PFN_XUnregisterIMInstantiateCallback)(Display *, void *, char *, char *, XIDProc, XPointer);
typedef int (*PFN_XResizeWindow)(Display *, XID, unsigned int, unsigned int);
typedef char *(*PFN_XResourceManagerString)(Display *);
typedef int (*PFN_XSaveContext)(Display *, XID, XContext, const char *);
typedef int (*PFN_XSelectInput)(Display *, XID, long);
typedef Status (*PFN_XSendEvent)(Display *, XID, Bool, long, XEvent *);
typedef int (*PFN_XSetClassHint)(Display *, XID, XClassHint *);
typedef XErrorHandler (*PFN_XSetErrorHandler)(XErrorHandler);
typedef void (*PFN_XSetICFocus)(XIC);
typedef char *(*PFN_XSetIMValues)(XIM, ...);
typedef int (*PFN_XSetInputFocus)(Display *, XID, int, Time);
typedef char *(*PFN_XSetLocaleModifiers)(const char *);
typedef int (*PFN_XSetScreenSaver)(Display *, int, int, int, int);
typedef int (*PFN_XSetSelectionOwner)(Display *, Atom, XID, Time);
typedef int (*PFN_XSetWMHints)(Display *, XID, XWMHints *);
typedef void (*PFN_XSetWMNormalHints)(Display *, XID, XSizeHints *);
typedef Status (*PFN_XSetWMProtocols)(Display *, XID, Atom *, int);
typedef Bool (*PFN_XSupportsLocale)(void);
typedef int (*PFN_XSync)(Display *, Bool);
typedef Bool (*PFN_XTranslateCoordinates)(Display *, XID, XID, int, int, int *, int *, XID *);
typedef int (*PFN_XUndefineCursor)(Display *, ::Window);
typedef int (*PFN_XUngrabPointer)(Display *, Time);
typedef int (*PFN_XUnmapWindow)(Display *, XID);
typedef void (*PFN_XUnsetICFocus)(XIC);
typedef VisualID (*PFN_XVisualIDFromVisual)(Visual *);
typedef int (*PFN_XWarpPointer)(Display *, XID, XID, int, int, unsigned int, unsigned int, int, int);

// XKB
typedef Status (*PFN_XkbGetState)(Display *, unsigned int, XkbStatePtr);
typedef Bool (*PFN_XkbQueryExtension)(Display *, int *, int *, int *, int *, int *);
typedef Bool (*PFN_XkbSelectEventDetails)(Display *, unsigned int, unsigned int, unsigned long, unsigned long);
typedef Bool (*PFN_XkbSetDetectableAutoRepeat)(Display *, Bool, Bool *);

// X Input Extension
typedef Status (*PFN_XIQueryVersion)(Display *, int *, int *);
typedef int (*PFN_XISelectEvents)(Display *, XID, XIEventMask *, int);

// XRandr
typedef XRRCrtcGamma *(*PFN_XRRAllocGamma)(int);
typedef void (*PFN_XRRFreeCrtcInfo)(XRRCrtcInfo *);
typedef void (*PFN_XRRFreeGamma)(XRRCrtcGamma *);
typedef void (*PFN_XRRFreeOutputInfo)(XRROutputInfo *);
typedef void (*PFN_XRRFreeScreenResources)(XRRScreenResources *);
typedef XRRCrtcGamma *(*PFN_XRRGetCrtcGamma)(Display *, RRCrtc);
typedef int (*PFN_XRRGetCrtcGammaSize)(Display *, RRCrtc);
typedef XRRCrtcInfo *(*PFN_XRRGetCrtcInfo)(Display *, XRRScreenResources *, RRCrtc);
typedef XRROutputInfo *(*PFN_XRRGetOutputInfo)(Display *, XRRScreenResources *, RROutput);
typedef RROutput (*PFN_XRRGetOutputPrimary)(Display *, XID);
typedef XRRScreenResources *(*PFN_XRRGetScreenResourcesCurrent)(Display *, XID);
typedef Bool (*PFN_XRRQueryExtension)(Display *, int *, int *);
typedef Status (*PFN_XRRQueryVersion)(Display *, int *, int *);
typedef void (*PFN_XRRSelectInput)(Display *, XID, int);
typedef Status (*PFN_XRRSetCrtcConfig)(Display *, XRRScreenResources *, RRCrtc, Time, int, int, RRMode, Rotation,
                                       RROutput *, int);
typedef void (*PFN_XRRSetCrtcGamma)(Display *, RRCrtc, XRRCrtcGamma *);
typedef int (*PFN_XRRUpdateConfiguration)(XEvent *);

// X Cursor
typedef XcursorImage *(*PFN_XcursorImageCreate)(int, int);
typedef void (*PFN_XcursorImageDestroy)(XcursorImage *);
typedef ::Cursor (*PFN_XcursorImageLoadCursor)(Display *, const XcursorImage *);
typedef ::Cursor (*PFN_XcursorLibraryLoadCursor)(Display *, const char *);

// XCB
typedef xcb_connection_t *(*PFN_XGetXCBConnection)(Display *);

// XRender
typedef Bool (*PFN_XRenderQueryExtension)(Display *, int *, int *);
typedef Status (*PFN_XRenderQueryVersion)(Display *dpy, int *, int *);
typedef XRenderPictFormat *(*PFN_XRenderFindVisualFormat)(Display *, Visual const *);

namespace awin
{
    namespace platform
    {
        namespace x11
        {
            struct XlibLoader
            {
                void *handle = nullptr;
                PFN_XAllocClassHint XAllocClassHint = nullptr;
                PFN_XAllocSizeHints XAllocSizeHints = nullptr;
                PFN_XAllocWMHints XAllocWMHints = nullptr;
                PFN_XChangeProperty XChangeProperty = nullptr;
                PFN_XChangeWindowAttributes XChangeWindowAttributes = nullptr;
                PFN_XCheckIfEvent XCheckIfEvent = nullptr;
                PFN_XCheckTypedWindowEvent XCheckTypedWindowEvent = nullptr;
                PFN_XCloseDisplay XCloseDisplay = nullptr;
                PFN_XCloseIM XCloseIM = nullptr;
                PFN_XConvertSelection XConvertSelection = nullptr;
                PFN_XCreateColormap XCreateColormap = nullptr;
                PFN_XCreateFontCursor XCreateFontCursor = nullptr;
                PFN_XCreateIC XCreateIC = nullptr;
                PFN_XCreateRegion XCreateRegion = nullptr;
                PFN_XCreateWindow XCreateWindow = nullptr;
                PFN_XDefineCursor XDefineCursor = nullptr;
                PFN_XDeleteContext XDeleteContext = nullptr;
                PFN_XDeleteProperty XDeleteProperty = nullptr;
                PFN_XDestroyIC XDestroyIC = nullptr;
                PFN_XDestroyRegion XDestroyRegion = nullptr;
                PFN_XDestroyWindow XDestroyWindow = nullptr;
                PFN_XDisplayKeycodes XDisplayKeycodes = nullptr;
                PFN_XEventsQueued XEventsQueued = nullptr;
                PFN_XFilterEvent XFilterEvent = nullptr;
                PFN_XFindContext XFindContext = nullptr;
                PFN_XFlush XFlush = nullptr;
                PFN_XFree XFree = nullptr;
                PFN_XGetAtomName XGetAtomName = nullptr;
                PFN_XFreeColormap XFreeColormap = nullptr;
                PFN_XFreeCursor XFreeCursor = nullptr;
                PFN_XFreeEventData XFreeEventData = nullptr;
                PFN_XGetErrorText XGetErrorText = nullptr;
                PFN_XGetEventData XGetEventData = nullptr;
                PFN_XGetICValues XGetICValues = nullptr;
                PFN_XGetIMValues XGetIMValues = nullptr;
                PFN_XGetInputFocus XGetInputFocus = nullptr;
                PFN_XGetKeyboardMapping XGetKeyboardMapping = nullptr;
                PFN_XGetScreenSaver XGetScreenSaver = nullptr;
                PFN_XGetSelectionOwner XGetSelectionOwner = nullptr;
                PFN_XGetVisualInfo XGetVisualInfo = nullptr;
                PFN_XGetWMNormalHints XGetWMNormalHints = nullptr;
                PFN_XGetWindowAttributes XGetWindowAttributes = nullptr;
                PFN_XGetWindowProperty XGetWindowProperty = nullptr;
                PFN_XGrabPointer XGrabPointer = nullptr;
                PFN_XIconifyWindow XIconifyWindow = nullptr;
                PFN_XInitThreads XInitThreads = nullptr;
                PFN_XInternAtom XInternAtom = nullptr;
                PFN_XLookupString XLookupString = nullptr;
                PFN_Xutf8LookupString Xutf8LookupString = nullptr;
                PFN_Xutf8SetWMProperties Xutf8SetWMProperties = nullptr;
                PFN_XMapRaised XMapRaised = nullptr;
                PFN_XMapWindow XMapWindow = nullptr;
                PFN_XMoveResizeWindow XMoveResizeWindow = nullptr;
                PFN_XMoveWindow XMoveWindow = nullptr;
                PFN_XNextEvent XNextEvent = nullptr;
                PFN_XOpenDisplay XOpenDisplay = nullptr;
                PFN_XOpenIM XOpenIM = nullptr;
                PFN_XPeekEvent XPeekEvent = nullptr;
                PFN_XPending XPending = nullptr;
                PFN_XQueryExtension XQueryExtension = nullptr;
                PFN_XQueryPointer XQueryPointer = nullptr;
                PFN_XQueryTree XQueryTree = nullptr;
                PFN_XRaiseWindow XRaiseWindow = nullptr;
                PFN_XRegisterIMInstantiateCallback XRegisterIMInstantiateCallback = nullptr;
                PFN_XUnregisterIMInstantiateCallback XUnregisterIMInstantiateCallback = nullptr;
                PFN_XResizeWindow XResizeWindow = nullptr;
                PFN_XResourceManagerString XResourceManagerString = nullptr;
                PFN_XrmDestroyDatabase XrmDestroyDatabase = nullptr;
                PFN_XrmGetResource XrmGetResource = nullptr;
                PFN_XrmGetStringDatabase XrmGetStringDatabase = nullptr;
                PFN_XrmInitialize XrmInitialize = nullptr;
                PFN_XrmUniqueQuark XrmUniqueQuark = nullptr;
                PFN_XSaveContext XSaveContext = nullptr;
                PFN_XSelectInput XSelectInput = nullptr;
                PFN_XSendEvent XSendEvent = nullptr;
                PFN_XSetClassHint XSetClassHint = nullptr;
                PFN_XSetErrorHandler XSetErrorHandler = nullptr;
                PFN_XSetICFocus XSetICFocus = nullptr;
                PFN_XSetIMValues XSetIMValues = nullptr;
                PFN_XSetInputFocus XSetInputFocus = nullptr;
                PFN_XSetLocaleModifiers XSetLocaleModifiers = nullptr;
                PFN_XSetScreenSaver XSetScreenSaver = nullptr;
                PFN_XSetSelectionOwner XSetSelectionOwner = nullptr;
                PFN_XSetWMHints XSetWMHints = nullptr;
                PFN_XSetWMNormalHints XSetWMNormalHints = nullptr;
                PFN_XSetWMProtocols XSetWMProtocols = nullptr;
                PFN_XSupportsLocale XSupportsLocale = nullptr;
                PFN_XSync XSync = nullptr;
                PFN_XTranslateCoordinates XTranslateCoordinates = nullptr;
                PFN_XUndefineCursor XUndefineCursor = nullptr;
                PFN_XUngrabPointer XUngrabPointer = nullptr;
                PFN_XUnmapWindow XUnmapWindow = nullptr;
                PFN_XUnsetICFocus XUnsetICFocus = nullptr;
                PFN_XVisualIDFromVisual XVisualIDFromVisual = nullptr;
                PFN_XWarpPointer XWarpPointer = nullptr;

                bool load();

                void unload()
                {
                    if (handle)
                    {
                        dlclose(handle);
                        handle = nullptr;
                    }
                }
            };

            struct XKBLoader
            {
                PFN_XkbGetState XkbGetState = nullptr;
                PFN_XkbQueryExtension XkbQueryExtension = nullptr;
                PFN_XkbSelectEventDetails XkbSelectEventDetails = nullptr;
                PFN_XkbSetDetectableAutoRepeat XkbSetDetectableAutoRepeat = nullptr;

                void load(void *handle);
            };

            struct XILoader
            {
            public:
                void *handle = nullptr;

                PFN_XIQueryVersion XIQueryVersion = nullptr;
                PFN_XISelectEvents XISelectEvents = nullptr;

                bool load();
            };

            class XCursorLoader
            {
            public:
                void *handle = nullptr;

                PFN_XcursorImageCreate XcursorImageCreate = nullptr;
                PFN_XcursorImageDestroy XcursorImageDestroy = nullptr;
                PFN_XcursorImageLoadCursor XcursorImageLoadCursor = nullptr;
                PFN_XcursorLibraryLoadCursor XcursorLibraryLoadCursor = nullptr;

                bool load();
            };

            class XCBLoader
            {
            public:
                void *handle = nullptr;

                PFN_XGetXCBConnection XGetXCBConnection = nullptr;

                bool load();
            };
        } // namespace x11
    } // namespace platform
} // namespace awin