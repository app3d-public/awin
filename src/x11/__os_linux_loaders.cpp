#include <X11/Xlib.h>
#include <acul/log.hpp>
#include "loaders.hpp"

#define LOAD_FUNCTION(name, handle) name = (PFN_##name)dlsym(handle, #name)

namespace awin
{
    namespace platform
    {
        namespace x11
        {
            bool X11Loader::load()
            {
#if defined(__CYGWIN__)
                xlib = dlopen("libX11-6.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
                xlib = dlopen("libX11.so", RTLD_LAZY);
#else
                xlib = dlopen("libX11.so.6", RTLD_LAZY);
#endif
                if (!xlib)
                {
                    LOG_ERROR("Failed to load X11 library: %s", dlerror());
                    return false;
                }

                LOAD_FUNCTION(XAllocClassHint, xlib);
                LOAD_FUNCTION(XAllocSizeHints, xlib);
                LOAD_FUNCTION(XAllocWMHints, xlib);
                LOAD_FUNCTION(XChangeProperty, xlib);
                LOAD_FUNCTION(XChangeWindowAttributes, xlib);
                LOAD_FUNCTION(XCheckIfEvent, xlib);
                LOAD_FUNCTION(XCheckTypedWindowEvent, xlib);
                LOAD_FUNCTION(XCloseDisplay, xlib);
                LOAD_FUNCTION(XCloseIM, xlib);
                LOAD_FUNCTION(XConvertSelection, xlib);
                LOAD_FUNCTION(XCreateColormap, xlib);
                LOAD_FUNCTION(XCreateFontCursor, xlib);
                LOAD_FUNCTION(XCreateIC, xlib);
                LOAD_FUNCTION(XCreateRegion, xlib);
                LOAD_FUNCTION(XCreateWindow, xlib);
                LOAD_FUNCTION(XDefineCursor, xlib);
                LOAD_FUNCTION(XDeleteContext, xlib);
                LOAD_FUNCTION(XDeleteProperty, xlib);
                LOAD_FUNCTION(XDestroyIC, xlib);
                LOAD_FUNCTION(XDestroyRegion, xlib);
                LOAD_FUNCTION(XDestroyWindow, xlib);
                LOAD_FUNCTION(XDisplayKeycodes, xlib);
                LOAD_FUNCTION(XEventsQueued, xlib);
                LOAD_FUNCTION(XFilterEvent, xlib);
                LOAD_FUNCTION(XFindContext, xlib);
                LOAD_FUNCTION(XFlush, xlib);
                LOAD_FUNCTION(XFree, xlib);
                LOAD_FUNCTION(XGetAtomName, xlib);
                LOAD_FUNCTION(XFreeColormap, xlib);
                LOAD_FUNCTION(XFreeCursor, xlib);
                LOAD_FUNCTION(XFreeEventData, xlib);
                LOAD_FUNCTION(XGetErrorText, xlib);
                LOAD_FUNCTION(XGetEventData, xlib);
                LOAD_FUNCTION(XGetICValues, xlib);
                LOAD_FUNCTION(XGetIMValues, xlib);
                LOAD_FUNCTION(XGetInputFocus, xlib);
                LOAD_FUNCTION(XGetKeyboardMapping, xlib);
                LOAD_FUNCTION(XGetScreenSaver, xlib);
                LOAD_FUNCTION(XGetSelectionOwner, xlib);
                LOAD_FUNCTION(XGetVisualInfo, xlib);
                LOAD_FUNCTION(XGetWMNormalHints, xlib);
                LOAD_FUNCTION(XGetWindowAttributes, xlib);
                LOAD_FUNCTION(XGetWindowProperty, xlib);
                LOAD_FUNCTION(XGrabPointer, xlib);
                LOAD_FUNCTION(XIconifyWindow, xlib);
                LOAD_FUNCTION(XInitThreads, xlib);
                LOAD_FUNCTION(XInternAtom, xlib);
                LOAD_FUNCTION(XLookupString, xlib);
                LOAD_FUNCTION(Xutf8LookupString, xlib);
                LOAD_FUNCTION(Xutf8SetWMProperties, xlib);
                LOAD_FUNCTION(XMapRaised, xlib);
                LOAD_FUNCTION(XMapWindow, xlib);
                LOAD_FUNCTION(XMoveResizeWindow, xlib);
                LOAD_FUNCTION(XMoveWindow, xlib);
                LOAD_FUNCTION(XNextEvent, xlib);
                LOAD_FUNCTION(XOpenDisplay, xlib);
                LOAD_FUNCTION(XOpenIM, xlib);
                LOAD_FUNCTION(XPeekEvent, xlib);
                LOAD_FUNCTION(XPending, xlib);
                LOAD_FUNCTION(XrmDestroyDatabase, xlib);
                LOAD_FUNCTION(XrmGetResource, xlib);
                LOAD_FUNCTION(XrmGetStringDatabase, xlib);
                LOAD_FUNCTION(XrmInitialize, xlib);
                LOAD_FUNCTION(XrmUniqueQuark, xlib);
                LOAD_FUNCTION(XQueryExtension, xlib);
                LOAD_FUNCTION(XQueryPointer, xlib);
                LOAD_FUNCTION(XQueryTree, xlib);
                LOAD_FUNCTION(XRaiseWindow, xlib);
                LOAD_FUNCTION(XRegisterIMInstantiateCallback, xlib);
                LOAD_FUNCTION(XUnregisterIMInstantiateCallback, xlib);
                LOAD_FUNCTION(XResizeWindow, xlib);
                LOAD_FUNCTION(XResourceManagerString, xlib);
                LOAD_FUNCTION(XSaveContext, xlib);
                LOAD_FUNCTION(XSelectInput, xlib);
                LOAD_FUNCTION(XSendEvent, xlib);
                LOAD_FUNCTION(XSetClassHint, xlib);
                LOAD_FUNCTION(XSetErrorHandler, xlib);
                LOAD_FUNCTION(XSetICFocus, xlib);
                LOAD_FUNCTION(XSetIMValues, xlib);
                LOAD_FUNCTION(XSetInputFocus, xlib);
                LOAD_FUNCTION(XSetLocaleModifiers, xlib);
                LOAD_FUNCTION(XSetScreenSaver, xlib);
                LOAD_FUNCTION(XSetSelectionOwner, xlib);
                LOAD_FUNCTION(XSetWMHints, xlib);
                LOAD_FUNCTION(XSetWMNormalHints, xlib);
                LOAD_FUNCTION(XSetWMProtocols, xlib);
                LOAD_FUNCTION(XSupportsLocale, xlib);
                LOAD_FUNCTION(XSync, xlib);
                LOAD_FUNCTION(XTranslateCoordinates, xlib);
                LOAD_FUNCTION(XUndefineCursor, xlib);
                LOAD_FUNCTION(XUngrabPointer, xlib);
                LOAD_FUNCTION(XUnmapWindow, xlib);
                LOAD_FUNCTION(XUnsetICFocus, xlib);
                LOAD_FUNCTION(XVisualIDFromVisual, xlib);
                LOAD_FUNCTION(XWarpPointer, xlib);
                LOAD_FUNCTION(XkbFreeKeyboard, xlib);
                LOAD_FUNCTION(XkbFreeNames, xlib);
                LOAD_FUNCTION(XkbGetMap, xlib);
                LOAD_FUNCTION(XkbGetNames, xlib);
                LOAD_FUNCTION(XkbGetState, xlib);
                LOAD_FUNCTION(XkbKeycodeToKeysym, xlib);
                LOAD_FUNCTION(XkbQueryExtension, xlib);
                LOAD_FUNCTION(XkbSelectEventDetails, xlib);
                LOAD_FUNCTION(XkbSetDetectableAutoRepeat, xlib);

                return true;
            }

            bool XILoader::load()
            {
#if defined(__CYGWIN__)
                xilib = dlopen("libXi-6.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
                xilib = dlopen("libXi.so", RTLD_LAZY);
#else
                xilib = dlopen("libXi.so.6", RTLD_LAZY);
#endif
                if (!xilib)
                {
                    LOG_ERROR("Failed to load X Input library: %s", dlerror());
                    return false;
                }

                LOAD_FUNCTION(XIQueryVersion, xilib);
                LOAD_FUNCTION(XISelectEvents, xilib);

                return true;
            }

            bool XRandrLoader::load()
            {
#if defined(__CYGWIN__)
                xrandr = dlopen("libXrandr-2.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
                xrandr = dlopen("libXrandr.so", RTLD_LAZY);
#else
                xrandr = dlopen("libXrandr.so.2", RTLD_LAZY);
#endif
                if (!xrandr)
                {
                    LOG_ERROR("Failed to load XRandr library: %s", dlerror());
                    return false;
                }

                LOAD_FUNCTION(XRRAllocGamma, xrandr);
                LOAD_FUNCTION(XRRFreeCrtcInfo, xrandr);
                LOAD_FUNCTION(XRRFreeGamma, xrandr);
                LOAD_FUNCTION(XRRFreeOutputInfo, xrandr);
                LOAD_FUNCTION(XRRFreeScreenResources, xrandr);
                LOAD_FUNCTION(XRRGetCrtcGamma, xrandr);
                LOAD_FUNCTION(XRRGetCrtcGammaSize, xrandr);
                LOAD_FUNCTION(XRRGetCrtcInfo, xrandr);
                LOAD_FUNCTION(XRRGetOutputInfo, xrandr);
                LOAD_FUNCTION(XRRGetOutputPrimary, xrandr);
                LOAD_FUNCTION(XRRGetScreenResourcesCurrent, xrandr);
                LOAD_FUNCTION(XRRQueryExtension, xrandr);
                LOAD_FUNCTION(XRRQueryVersion, xrandr);
                LOAD_FUNCTION(XRRSelectInput, xrandr);
                LOAD_FUNCTION(XRRSetCrtcConfig, xrandr);
                LOAD_FUNCTION(XRRSetCrtcGamma, xrandr);
                LOAD_FUNCTION(XRRUpdateConfiguration, xrandr);

                return true;
            }

            bool XCursorLoader::load()
            {
#if defined(__CYGWIN__)
                xcursor = dlopen("libXcursor-1.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
                xcursor = dlopen("libXcursor.so", RTLD_LAZY);
#else
                xcursor = dlopen("libXcursor.so.1", RTLD_LAZY);
#endif
                if (!xcursor)
                {
                    LOG_WARN("Failed to load X Cursor library: %s", dlerror());
                    return false;
                }

                LOAD_FUNCTION(XcursorImageCreate, xcursor);
                LOAD_FUNCTION(XcursorImageDestroy, xcursor);
                LOAD_FUNCTION(XcursorImageLoadCursor, xcursor);
                LOAD_FUNCTION(XcursorGetTheme, xcursor);
                LOAD_FUNCTION(XcursorGetDefaultSize, xcursor);
                LOAD_FUNCTION(XcursorLibraryLoadImage, xcursor);
                LOAD_FUNCTION(XcursorLibraryLoadCursor, xcursor);
                return true;
            }

            bool XCBLoader::load()
            {
#if defined(__CYGWIN__)
                xcb = dlopen("libX11-xcb-1.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
                xcb = dlopen("libX11-xcb.so", RTLD_LAZY);
#else
                xcb = dlopen("libX11-xcb.so.1", RTLD_LAZY);
#endif
                if (!xcb)
                {
                    LOG_WARN("Failed to load XCB library: %s", dlerror());
                    return false;
                }

                LOAD_FUNCTION(XGetXCBConnection, xcb);

                return true;
            }

            bool XRenderLoader::load()
            {
#if defined(__CYGWIN__)
                xrender = dlopen("libXrender-1.so", RTLD_LAZY);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
                xrender = dlopen("libXrender.so", RTLD_LAZY);
#else
                xrender = dlopen("libXrender.so.1", RTLD_LAZY);
#endif
                if (!xrender)
                {
                    LOG_WARN("Failed to load X Render library: %s", dlerror());
                    return false;
                }

                LOAD_FUNCTION(XRenderQueryExtension, xrender);
                LOAD_FUNCTION(XRenderQueryVersion, xrender);
                LOAD_FUNCTION(XRenderFindVisualFormat, xrender);

                return true;
            }
        } // namespace x11
    } // namespace platform
} // namespace awin