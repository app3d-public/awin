#include <acul/log.hpp>
#include <awin/window.hpp>

namespace awin
{
    DefaultRegistry eventRegistry(nullptr);
    namespace platform
    {
        WindowEnvironment env;

        void inputKey(WindowData *data, io::Key key, io::KeyPressState action, io::KeyMode mods)
        {
            if (+key >= 0 && key <= io::Key::kLast)
            {
                bool repeated{false};
                if (action == io::KeyPressState::release && data->keys[+key] == io::KeyPressState::release) return;
                if (action == io::KeyPressState::press && data->keys[+key] == io::KeyPressState::press) repeated = true;
                data->keys[+key] = action;
                if (repeated) action = io::KeyPressState::repeat;
            }
            dispatchWindowEvent(eventRegistry.keyInput, data->owner, key, action, mods);
        }
    } // namespace platform

    Cursor &Cursor::operator=(Cursor &&other) noexcept
    {
        if (this != &other)
        {
            _platform = other._platform;
            other._platform.cursor = NULL;
        }
        return *this;
    }

    void updateEvents()
    {
        assert(platform::env.ed);
#ifdef _WIN32
        auto NCLMouseDownList = platform::env.ed->get_listeners<Win32NativeEvent>(event_id::NCMouseDown);
        if (!NCLMouseDownList.empty()) eventRegistry.NCLMouseDown = NCLMouseDownList[0];

        auto NCHitTestList = platform::env.ed->get_listeners<Win32NativeEvent>(event_id::NCHitTest);
        if (!NCHitTestList.empty()) eventRegistry.NCHitTest = NCHitTestList[0];
#endif
        eventRegistry.focus = platform::env.ed->get_listeners<FocusEvent>(event_id::focus);
        eventRegistry.scroll = platform::env.ed->get_listeners<ScrollEvent>(event_id::scroll);
        eventRegistry.minimize = platform::env.ed->get_listeners<StateEvent>(event_id::minimize);
        eventRegistry.maximize = platform::env.ed->get_listeners<StateEvent>(event_id::maximize);
        eventRegistry.resize = platform::env.ed->get_listeners<PosEvent>(event_id::resize);
        eventRegistry.move = platform::env.ed->get_listeners<PosEvent>(event_id::move);
        eventRegistry.charInput = platform::env.ed->get_listeners<CharInputEvent>(event_id::charInput);
        eventRegistry.keyInput = platform::env.ed->get_listeners<KeyInputEvent>(event_id::keyInput);
        eventRegistry.mouseClick = platform::env.ed->get_listeners<MouseClickEvent>(event_id::mouseClick);
        eventRegistry.mouseEnter = platform::env.ed->get_listeners<MouseEnterEvent>(event_id::mouseEnter);
        eventRegistry.mouseMove = platform::env.ed->get_listeners<PosEvent>(event_id::mouseMove);
        eventRegistry.mouseMoveAbs = platform::env.ed->get_listeners<PosEvent>(event_id::mouseMoveAbs);
        eventRegistry.dpiChanged = platform::env.ed->get_listeners<DpiChangedEvent>(event_id::dpiChanged);
    }

    void initLibrary(acul::events::dispatcher *ed)
    {
        if (!platform::initPlatform()) throw acul::runtime_error("Failed to initialize Window platform");
        platform::initTimer();
        setTime(0.0);
        platform::env.ed = ed;
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
} // namespace awin