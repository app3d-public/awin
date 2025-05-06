#include <acul/log.hpp>
#include <awin/window.hpp>

namespace awin
{
    DefaultRegistry event_registry(nullptr);
    namespace platform
    {
        WindowEnvironment env;

        void input_key(WindowData *data, io::Key key, io::KeyPressState action, io::KeyMode mods)
        {
            if (+key >= 0 && key <= io::Key::Last)
            {
                bool repeated{false};
                if (action == io::KeyPressState::Release && data->keys[+key] == io::KeyPressState::Release) return;
                if (action == io::KeyPressState::Press && data->keys[+key] == io::KeyPressState::Press) repeated = true;
                data->keys[+key] = action;
                if (repeated) action = io::KeyPressState::Repeat;
            }
            dispatch_window_event(event_registry.key_input, data->owner, key, action, mods);
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

    void update_events()
    {
        assert(platform::env.ed);
#ifdef _WIN32
        auto NCLMouseDownList = platform::env.ed->get_listeners<Win32NativeEvent>(event_id::NCMouseDown);
        if (!NCLMouseDownList.empty()) event_registry.ncl_mouse_down = NCLMouseDownList[0];

        auto NCHitTestList = platform::env.ed->get_listeners<Win32NativeEvent>(event_id::NCHitTest);
        if (!NCHitTestList.empty()) event_registry.nc_hit_test = NCHitTestList[0];
#endif
        event_registry.focus = platform::env.ed->get_listeners<FocusEvent>(event_id::Focus);
        event_registry.scroll = platform::env.ed->get_listeners<ScrollEvent>(event_id::Scroll);
        event_registry.minimize = platform::env.ed->get_listeners<StateEvent>(event_id::Minimize);
        event_registry.maximize = platform::env.ed->get_listeners<StateEvent>(event_id::Maximize);
        event_registry.resize = platform::env.ed->get_listeners<PosEvent>(event_id::Resize);
        event_registry.move = platform::env.ed->get_listeners<PosEvent>(event_id::Move);
        event_registry.char_input = platform::env.ed->get_listeners<CharInputEvent>(event_id::CharInput);
        event_registry.key_input = platform::env.ed->get_listeners<KeyInputEvent>(event_id::KeyInput);
        event_registry.mouse_click = platform::env.ed->get_listeners<MouseClickEvent>(event_id::MouseClick);
        event_registry.mouse_enter = platform::env.ed->get_listeners<MouseEnterEvent>(event_id::MouseEnter);
        event_registry.mouse_move = platform::env.ed->get_listeners<PosEvent>(event_id::MouseMove);
        event_registry.mouse_move_abs = platform::env.ed->get_listeners<PosEvent>(event_id::MouseMoveAbs);
        event_registry.dpi_changed = platform::env.ed->get_listeners<DpiChangedEvent>(event_id::DpiChanged);
    }

    void init_library(acul::events::dispatcher *ed)
    {
        if (!platform::init_platform()) throw acul::runtime_error("Failed to initialize Window platform");
        platform::init_timer();
        set_time(0.0);
        platform::env.ed = ed;
    }

    void destroy_library()
    {
        LOG_INFO("Destroying Window platform");
        platform::destroy_platform();
    }

    f64 get_time()
    {
        return static_cast<f64>(platform::get_time_value() - platform::env.timer.offset) /
               platform::get_time_frequency();
    }

    void set_time(f64 time)
    {
        if (std::isnan(time) || time < 0.0 || time > 18446744073.0)
        {
            LOG_ERROR("Invalid time value: %f", time);
            return;
        }
        platform::env.timer.offset =
            platform::get_time_value() - static_cast<u64>(time * platform::get_time_frequency());
    }
} // namespace awin