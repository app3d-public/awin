#include <window/window.hpp>

namespace window
{
    void updateEvents()
    {
        using namespace _internal;
#ifdef _WIN32
        auto NCLMouseClickList = gWindowEvents.e->getListeners<Win32NativeEvent>("window:NCLMouseClick");
        if (!NCLMouseClickList.empty())
            gWindowEvents.NCLMouseClick = NCLMouseClickList[0].get();

        auto NCHitTestList = gWindowEvents.e->getListeners<Win32NativeEvent>("window:NCHitTest");
        if (!NCHitTestList.empty())
            gWindowEvents.NCHitTest = NCHitTestList[0].get();
#endif
        gWindowEvents.focusEvents = gWindowEvents.e->getListeners<FocusEvent>("window:focus");
        gWindowEvents.charInputEvents = gWindowEvents.e->getListeners<CharInputEvent>("window:charInput");
        gWindowEvents.keyInputEvents = gWindowEvents.e->getListeners<KeyInputEvent>("window:keyInput");
        gWindowEvents.mouseClickEvents = gWindowEvents.e->getListeners<MouseClickEvent>("window:mouseClick");
        gWindowEvents.cursorEnterEvents = gWindowEvents.e->getListeners<CursorEnterEvent>("window:cursorEnter");
        gWindowEvents.cursorPosEvents = gWindowEvents.e->getListeners<PosEvent>("window:mouseMove");
        gWindowEvents.scrollEvents = gWindowEvents.e->getListeners<ScrollEvent>("window:scroll");
        gWindowEvents.minimizeEvents = gWindowEvents.e->getListeners<PosEvent>("window:minimize");
        gWindowEvents.maximizeEvents = gWindowEvents.e->getListeners<PosEvent>("window:maximize");
        gWindowEvents.resizeEvents = gWindowEvents.e->getListeners<PosEvent>("window:resize");
        gWindowEvents.moveEvents = gWindowEvents.e->getListeners<PosEvent>("window:move");
        gWindowEvents.dpiChangedEvents = gWindowEvents.e->getListeners<DpiChangedEvent>("window:dpiChanged");
    }
} // namespace window