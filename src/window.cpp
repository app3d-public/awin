#include <core/log.hpp>
#include <window/window.hpp>

namespace window
{
    DefaultRegistry eventRegistry(nullptr);
    namespace platform
    {
        WindowEnvironment env{nullptr};

        void inputKey(WindowPlatformData *impl, io::Key key, io::KeyPressState action, io::KeyMode mods)
        {
            if (+key >= 0 && key <= io::Key::kLast)
            {
                bool repeated{false};
                if (action == io::KeyPressState::release && impl->keys[+key] == io::KeyPressState::release) return;
                if (action == io::KeyPressState::press && impl->keys[+key] == io::KeyPressState::press) repeated = true;
                impl->keys[+key] = action;
                if (repeated) action = io::KeyPressState::repeat;
            }
            dispatchWindowEvent(eventRegistry.keyInputEvents, "window:input:key", impl->owner, key, action, mods);
        }
    } // namespace platform

    Cursor &Cursor::operator=(Cursor &&other) noexcept
    {
        if (this != &other)
        {
            astl::release(_platform);
            _platform = other._platform;
            other._platform = nullptr;
        }
        return *this;
    }

    Cursor::~Cursor() { astl::release(_platform); }

    void updateEvents()
    {
        assert(platform::env.e);
#ifdef _WIN32
        auto NCLMouseDownList = platform::env.e->getListeners<Win32NativeEvent>("window:NCLMouseDown");
        if (!NCLMouseDownList.empty()) eventRegistry.NCLMouseDown = NCLMouseDownList[0];

        auto NCHitTestList = platform::env.e->getListeners<Win32NativeEvent>("window:NCHitTest");
        if (!NCHitTestList.empty()) eventRegistry.NCHitTest = NCHitTestList[0];
#endif
        eventRegistry.focusEvents = platform::env.e->getListeners<FocusEvent>("window:focus");
        eventRegistry.scrollEvents = platform::env.e->getListeners<ScrollEvent>("window:scroll");
        eventRegistry.minimizeEvents = platform::env.e->getListeners<StateEvent>("window:minimize");
        eventRegistry.maximizeEvents = platform::env.e->getListeners<StateEvent>("window:maximize");
        eventRegistry.resizeEvents = platform::env.e->getListeners<PosEvent>("window:resize");
        eventRegistry.moveEvents = platform::env.e->getListeners<PosEvent>("window:move");
        eventRegistry.charInputEvents = platform::env.e->getListeners<CharInputEvent>("window:input:char");
        eventRegistry.keyInputEvents = platform::env.e->getListeners<KeyInputEvent>("window:input:key");
        eventRegistry.mouseClickEvents = platform::env.e->getListeners<MouseClickEvent>("window:input:mouse");
        eventRegistry.cursorEnterEvents = platform::env.e->getListeners<CursorEnterEvent>("window:cursor:enter");
        eventRegistry.cursorPosEvents = platform::env.e->getListeners<PosEvent>("window:cursor:move");
        eventRegistry.cursorPosAbsEvents = platform::env.e->getListeners<PosEvent>("window:cursor:move:abs");
        eventRegistry.dpiChangedEvents = platform::env.e->getListeners<DpiChangedEvent>("window:dpiChanged");
    }

    void initLibrary(events::Manager *e)
    {
        if (!platform::initPlatform()) throw std::runtime_error("Failed to initialize Window platform");
        platform::initTimer();
        setTime(0.0);
        platform::env.e = e;
    }

    void destroyLibrary()
    {
        logInfo("Destroying Window platform");
        platform::destroyPlatform();
    }

    f64 getTime()
    {
        return static_cast<f64>(platform::getTimeValue() - platform::env.timer.offset) / platform::getTimeFrequency();
    }

    void setTime(f64 time)
    {
        if (std::isnan(time) || time < 0.0 || time > 18446744073.0)
        {
            logError("Invalid time value: %f", time);
            return;
        }
        platform::env.timer.offset = platform::getTimeValue() - static_cast<u64>(time * platform::getTimeFrequency());
    }
} // namespace window