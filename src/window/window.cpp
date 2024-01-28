#include <core/log.hpp>
#include <window/window.hpp>
#ifdef _WIN32
    #include <window/platform_win32.hpp>
#else
    #error "Unsupported platform"
#endif

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
                if (action == io::KeyPressState::release && impl->keys[+key] == io::KeyPressState::release)
                    return;
                if (action == io::KeyPressState::press && impl->keys[+key] == io::KeyPressState::press)
                    repeated = true;
                impl->keys[+key] = action;
                if (repeated)
                    action = io::KeyPressState::repeat;
            }
            emitWindowEvent(eventRegistry.keyInputEvents, "window:input:key", impl->owner, key, action, mods);
        }
    } // namespace platform

    Cursor::~Cursor()
    {
        if (_platform)
            delete _platform;
    }

    Cursor &Cursor::operator=(Cursor &&other) noexcept
    {
        if (this != &other)
        {
            delete _platform;
            _platform = other._platform;
            other._platform = nullptr;
        }
        return *this;
    }

    Point2D Window::dimensions() const { return _platform->dimenstions; }

    bool Window::decorated() const { return (_platform->flags & window::CreationFlagsBits::decorated) != 0; }

    bool Window::resizable() const { return (_platform->flags & window::CreationFlagsBits::resizable) != 0; }

    bool Window::fullscreen() const { return (_platform->flags & window::CreationFlagsBits::fullscreen) != 0; }

    void Window::setCursor(Cursor *cursor) { _platform->cursor = cursor; }

    bool Window::focused() const { return _platform->focused; }

    bool Window::minimized() const { return _platform->minimized; }

    bool Window::maximized() const { return _platform->maximized; }

    bool Window::hidden() const { return (_platform->flags & window::CreationFlagsBits::hidden) != 0; }

    Point2D Window::resizeLimit() const { return _platform->resizeLimit; }

    void Window::resizeLimit(i32 width, i32 height) { _platform->resizeLimit = {width, height}; }

    bool Window::readyToClose() const { return _platform->readyToClose; }

    void updateEvents()
    {
#ifdef _WIN32
        auto NCLMouseClickList = eventRegistry.mng->getListeners<Win32NativeEvent>("window:NCLMouseClick");
        if (!NCLMouseClickList.empty())
            eventRegistry.NCLMouseClick = NCLMouseClickList[0].get();

        auto NCHitTestList = eventRegistry.mng->getListeners<Win32NativeEvent>("window:NCHitTest");
        if (!NCHitTestList.empty())
            eventRegistry.NCHitTest = NCHitTestList[0].get();
#endif
        eventRegistry.focusEvents = eventRegistry.mng->getListeners<FocusEvent>("window:focus");
        eventRegistry.scrollEvents = eventRegistry.mng->getListeners<ScrollEvent>("window:scroll");
        eventRegistry.minimizeEvents = eventRegistry.mng->getListeners<PosEvent>("window:minimize");
        eventRegistry.maximizeEvents = eventRegistry.mng->getListeners<PosEvent>("window:maximize");
        eventRegistry.resizeEvents = eventRegistry.mng->getListeners<PosEvent>("window:resize");
        eventRegistry.moveEvents = eventRegistry.mng->getListeners<PosEvent>("window:move");
        eventRegistry.charInputEvents = eventRegistry.mng->getListeners<CharInputEvent>("window:input:char");
        eventRegistry.keyInputEvents = eventRegistry.mng->getListeners<KeyInputEvent>("window:input:key");
        eventRegistry.mouseClickEvents = eventRegistry.mng->getListeners<MouseClickEvent>("window:input:mouse");
        eventRegistry.cursorEnterEvents = eventRegistry.mng->getListeners<CursorEnterEvent>("window:cursor:enter");
        eventRegistry.cursorPosEvents = eventRegistry.mng->getListeners<PosEvent>("window:cursor:move");
        eventRegistry.cursorPosAbsEvents = eventRegistry.mng->getListeners<PosEvent>("window:cursor:move:abs");
        eventRegistry.dpiChangedEvents = eventRegistry.mng->getListeners<DpiChangedEvent>("window:dpiChanged");
    }

    void initLibrary(EventManager &events)
    {
        if (!platform::initPlatform())
            throw std::runtime_error("Failed to initialize Window platform");
        eventRegistry.mng = &events;
        platform::initTimer();
        setTime(0.0);
    }

    void destroyLibrary()
    {
        logInfo("Destroying Window platform");
        platform::destroyPlatform();
        eventRegistry.mng = nullptr;
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