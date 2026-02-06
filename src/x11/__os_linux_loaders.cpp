#include <X11/Xlib.h>
#include "../env.hpp"
#include "loaders.hpp"

#define LOAD_FUNCTION(name, handle) name = (PFN_##name)dlsym(handle, #name)

namespace awin::platform::x11
{
    bool XlibLoader::load()
    {
#if defined(__CYGWIN__)
        handle = dlopen("libX11-6.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        handle = dlopen("libX11.so", RTLD_LAZY);
#else
        handle = dlopen("libX11.so.6", RTLD_LAZY);
#endif
        if (!handle)
        {
            AWIN_LOG_ERROR("Failed to load X11 library: %s", dlerror());
            return false;
        }

        LOAD_FUNCTION(XAllocClassHint, handle);
        LOAD_FUNCTION(XAllocSizeHints, handle);
        LOAD_FUNCTION(XAllocWMHints, handle);
        LOAD_FUNCTION(XChangeProperty, handle);
        LOAD_FUNCTION(XChangeWindowAttributes, handle);
        LOAD_FUNCTION(XCheckIfEvent, handle);
        LOAD_FUNCTION(XCheckTypedWindowEvent, handle);
        LOAD_FUNCTION(XCloseDisplay, handle);
        LOAD_FUNCTION(XCloseIM, handle);
        LOAD_FUNCTION(XConvertSelection, handle);
        LOAD_FUNCTION(XCreateColormap, handle);
        LOAD_FUNCTION(XCreateFontCursor, handle);
        LOAD_FUNCTION(XCreateIC, handle);
        LOAD_FUNCTION(XCreateRegion, handle);
        LOAD_FUNCTION(XCreateWindow, handle);
        LOAD_FUNCTION(XDefineCursor, handle);
        LOAD_FUNCTION(XDeleteContext, handle);
        LOAD_FUNCTION(XDeleteProperty, handle);
        LOAD_FUNCTION(XDestroyIC, handle);
        LOAD_FUNCTION(XDestroyRegion, handle);
        LOAD_FUNCTION(XDestroyWindow, handle);
        LOAD_FUNCTION(XDisplayKeycodes, handle);
        LOAD_FUNCTION(XEventsQueued, handle);
        LOAD_FUNCTION(XFilterEvent, handle);
        LOAD_FUNCTION(XFindContext, handle);
        LOAD_FUNCTION(XFlush, handle);
        LOAD_FUNCTION(XFree, handle);
        LOAD_FUNCTION(XGetAtomName, handle);
        LOAD_FUNCTION(XFreeColormap, handle);
        LOAD_FUNCTION(XFreeCursor, handle);
        LOAD_FUNCTION(XFreeEventData, handle);
        LOAD_FUNCTION(XGetErrorText, handle);
        LOAD_FUNCTION(XGetEventData, handle);
        LOAD_FUNCTION(XGetICValues, handle);
        LOAD_FUNCTION(XGetIMValues, handle);
        LOAD_FUNCTION(XGetInputFocus, handle);
        LOAD_FUNCTION(XGetKeyboardMapping, handle);
        LOAD_FUNCTION(XGetScreenSaver, handle);
        LOAD_FUNCTION(XGetSelectionOwner, handle);
        LOAD_FUNCTION(XGetVisualInfo, handle);
        LOAD_FUNCTION(XGetWMNormalHints, handle);
        LOAD_FUNCTION(XGetWindowAttributes, handle);
        LOAD_FUNCTION(XGetWindowProperty, handle);
        LOAD_FUNCTION(XGrabPointer, handle);
        LOAD_FUNCTION(XIconifyWindow, handle);
        LOAD_FUNCTION(XInitThreads, handle);
        LOAD_FUNCTION(XInternAtom, handle);
        LOAD_FUNCTION(XLookupString, handle);
        LOAD_FUNCTION(Xutf8LookupString, handle);
        LOAD_FUNCTION(Xutf8SetWMProperties, handle);
        LOAD_FUNCTION(XMapRaised, handle);
        LOAD_FUNCTION(XMapWindow, handle);
        LOAD_FUNCTION(XMoveResizeWindow, handle);
        LOAD_FUNCTION(XMoveWindow, handle);
        LOAD_FUNCTION(XNextEvent, handle);
        LOAD_FUNCTION(XOpenDisplay, handle);
        LOAD_FUNCTION(XOpenIM, handle);
        LOAD_FUNCTION(XPeekEvent, handle);
        LOAD_FUNCTION(XPending, handle);
        LOAD_FUNCTION(XrmDestroyDatabase, handle);
        LOAD_FUNCTION(XrmGetResource, handle);
        LOAD_FUNCTION(XrmGetStringDatabase, handle);
        LOAD_FUNCTION(XrmInitialize, handle);
        LOAD_FUNCTION(XrmUniqueQuark, handle);
        LOAD_FUNCTION(XQueryExtension, handle);
        LOAD_FUNCTION(XQueryPointer, handle);
        LOAD_FUNCTION(XQueryTree, handle);
        LOAD_FUNCTION(XRaiseWindow, handle);
        LOAD_FUNCTION(XRegisterIMInstantiateCallback, handle);
        LOAD_FUNCTION(XUnregisterIMInstantiateCallback, handle);
        LOAD_FUNCTION(XResizeWindow, handle);
        LOAD_FUNCTION(XResourceManagerString, handle);
        LOAD_FUNCTION(XSaveContext, handle);
        LOAD_FUNCTION(XSelectInput, handle);
        LOAD_FUNCTION(XSendEvent, handle);
        LOAD_FUNCTION(XSetClassHint, handle);
        LOAD_FUNCTION(XSetErrorHandler, handle);
        LOAD_FUNCTION(XSetICFocus, handle);
        LOAD_FUNCTION(XSetIMValues, handle);
        LOAD_FUNCTION(XSetInputFocus, handle);
        LOAD_FUNCTION(XSetLocaleModifiers, handle);
        LOAD_FUNCTION(XSetScreenSaver, handle);
        LOAD_FUNCTION(XSetSelectionOwner, handle);
        LOAD_FUNCTION(XSetWMHints, handle);
        LOAD_FUNCTION(XSetWMNormalHints, handle);
        LOAD_FUNCTION(XSetWMProtocols, handle);
        LOAD_FUNCTION(XSupportsLocale, handle);
        LOAD_FUNCTION(XSync, handle);
        LOAD_FUNCTION(XTranslateCoordinates, handle);
        LOAD_FUNCTION(XUndefineCursor, handle);
        LOAD_FUNCTION(XUngrabPointer, handle);
        LOAD_FUNCTION(XUnmapWindow, handle);
        LOAD_FUNCTION(XUnsetICFocus, handle);
        LOAD_FUNCTION(XVisualIDFromVisual, handle);
        LOAD_FUNCTION(XWarpPointer, handle);
        return true;
    }

    void XKBLoader::load(void *handle)
    {
        LOAD_FUNCTION(XkbGetState, handle);
        LOAD_FUNCTION(XkbQueryExtension, handle);
        LOAD_FUNCTION(XkbSelectEventDetails, handle);
        LOAD_FUNCTION(XkbSetDetectableAutoRepeat, handle);
    }

    bool XILoader::load()
    {
#if defined(__CYGWIN__)
        handle = dlopen("libXi-6.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        handle = dlopen("libXi.so", RTLD_LAZY);
#else
        handle = dlopen("libXi.so.6", RTLD_LAZY);
#endif
        if (!handle)
        {
            AWIN_LOG_ERROR("Failed to load X Input library: %s", dlerror());
            return false;
        }

        LOAD_FUNCTION(XIQueryVersion, handle);
        LOAD_FUNCTION(XISelectEvents, handle);

        return true;
    }

    bool XCursorLoader::load()
    {
#if defined(__CYGWIN__)
        handle = dlopen("libXcursor-1.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        handle = dlopen("libXcursor.so", RTLD_LAZY);
#else
        handle = dlopen("libXcursor.so.1", RTLD_LAZY);
#endif
        if (!handle)
        {
            AWIN_LOG_WARN("Failed to load X Cursor library: %s", dlerror());
            return false;
        }

        LOAD_FUNCTION(XcursorImageCreate, handle);
        LOAD_FUNCTION(XcursorImageDestroy, handle);
        LOAD_FUNCTION(XcursorImageLoadCursor, handle);
        LOAD_FUNCTION(XcursorLibraryLoadCursor, handle);
        return true;
    }

    bool XCBLoader::load()
    {
#if defined(__CYGWIN__)
        handle = dlopen("libX11-xcb-1.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        handle = dlopen("libX11-xcb.so", RTLD_LAZY);
#else
        handle = dlopen("libX11-xcb.so.1", RTLD_LAZY);
#endif
        if (!handle)
        {
            AWIN_LOG_WARN("Failed to load XCB library: %s", dlerror());
            return false;
        }

        LOAD_FUNCTION(XGetXCBConnection, handle);

        return true;
    }
} // namespace awin::platform::x11