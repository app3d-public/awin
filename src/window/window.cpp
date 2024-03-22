#include <core/log.hpp>
#include <window/window.hpp>
#ifdef _WIN32
    #include <window/platform@win32.hpp>
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

    bool Window::isCursorHidden() const { return _platform->isCursorHidden; }

    bool Window::focused() const { return _platform->focused; }

    bool Window::minimized() const { return _platform->flags & window::CreationFlagsBits::minimized; }

    bool Window::maximized() const { return _platform->flags & window::CreationFlagsBits::maximized; }

    bool Window::hidden() const { return (_platform->flags & window::CreationFlagsBits::hidden) != 0; }

    Point2D Window::resizeLimit() const { return _platform->resizeLimit; }

    void Window::resizeLimit(i32 width, i32 height) { _platform->resizeLimit = {width, height}; }

    bool Window::readyToClose() const { return _platform->readyToClose; }

    void Window::close() { _platform->readyToClose = true; }

    void updateEvents()
    {
#ifdef _WIN32
        auto NCLMouseDownList = events::mng.getListeners<Win32NativeEvent>("window:NCLMouseDown");
        if (!NCLMouseDownList.empty())
            eventRegistry.NCLMouseDown = NCLMouseDownList[0].get();

        auto NCHitTestList = events::mng.getListeners<Win32NativeEvent>("window:NCHitTest");
        if (!NCHitTestList.empty())
            eventRegistry.NCHitTest = NCHitTestList[0].get();
#endif
        eventRegistry.focusEvents = events::mng.getListeners<FocusEvent>("window:focus");
        eventRegistry.scrollEvents = events::mng.getListeners<ScrollEvent>("window:scroll");
        eventRegistry.minimizeEvents = events::mng.getListeners<StateEvent>("window:minimize");
        eventRegistry.maximizeEvents = events::mng.getListeners<StateEvent>("window:maximize");
        eventRegistry.resizeEvents = events::mng.getListeners<PosEvent>("window:resize");
        eventRegistry.moveEvents = events::mng.getListeners<PosEvent>("window:move");
        eventRegistry.charInputEvents = events::mng.getListeners<CharInputEvent>("window:input:char");
        eventRegistry.keyInputEvents = events::mng.getListeners<KeyInputEvent>("window:input:key");
        eventRegistry.mouseClickEvents = events::mng.getListeners<MouseClickEvent>("window:input:mouse");
        eventRegistry.cursorEnterEvents = events::mng.getListeners<CursorEnterEvent>("window:cursor:enter");
        eventRegistry.cursorPosEvents = events::mng.getListeners<PosEvent>("window:cursor:move");
        eventRegistry.cursorPosAbsEvents = events::mng.getListeners<PosEvent>("window:cursor:move:abs");
        eventRegistry.dpiChangedEvents = events::mng.getListeners<DpiChangedEvent>("window:dpiChanged");
    }

    void initLibrary()
    {
        if (!platform::initPlatform())
            throw std::runtime_error("Failed to initialize Window platform");
        platform::initTimer();
        setTime(0.0);
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