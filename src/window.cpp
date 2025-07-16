#include <acul/log.hpp>
#include <awin/window.hpp>
#include "env.hpp"

namespace awin
{
    namespace platform
    {
        EventRegistry event_registry;
        WindowEnvironment env;

        void input_key(WindowData *data, io::Key key, io::KeyPressState action, io::KeyMode mods)
        {
            if (+key >= 0 && key <= io::Key::last)
            {
                bool repeated{false};
                if (action == io::KeyPressState::release && data->keys[+key] == io::KeyPressState::release) return;
                if (action == io::KeyPressState::press && data->keys[+key] == io::KeyPressState::press) repeated = true;
                data->keys[+key] = action;
                if (repeated) action = io::KeyPressState::repeat;
            }
            dispatch_window_event(event_registry.key_input, data->owner, key, action, mods);
        }
    } // namespace platform

    Cursor &Cursor::operator=(Cursor &&other) noexcept
    {
        if (this != &other)
        {
            if (_pd != other._pd) reset();

            _pd = other._pd;
            other._pd = nullptr;
        }
        return *this;
    }

    void update_events()
    {
        assert(platform::env.ed);
#ifdef _WIN32
        auto nc_mouse_down = platform::env.ed->get_listeners<Win32NativeEvent>(event_id::nc_mouse_down);
        if (!nc_mouse_down.empty()) event_registry.ncl_mouse_down = NCLMouseDownList[0];

        auto nc_hit_test = platform::env.ed->get_listeners<Win32NativeEvent>(event_id::nc_hit_test);
        if (!nc_hit_test.empty()) event_registry.nc_hit_test = NCHitTestList[0];
#endif
        platform::event_registry.focus = platform::env.ed->get_listeners<FocusEvent>(event_id::focus);
        platform::event_registry.scroll = platform::env.ed->get_listeners<ScrollEvent>(event_id::scroll);
        platform::event_registry.minimize = platform::env.ed->get_listeners<StateEvent>(event_id::minimize);
        platform::event_registry.maximize = platform::env.ed->get_listeners<StateEvent>(event_id::maximize);
        platform::event_registry.resize = platform::env.ed->get_listeners<PosEvent>(event_id::resize);
        platform::event_registry.move = platform::env.ed->get_listeners<PosEvent>(event_id::move);
        platform::event_registry.char_input = platform::env.ed->get_listeners<CharInputEvent>(event_id::char_input);
        platform::event_registry.key_input = platform::env.ed->get_listeners<KeyInputEvent>(event_id::key_input);
        platform::event_registry.mouse_click = platform::env.ed->get_listeners<MouseClickEvent>(event_id::mouse_click);
        platform::event_registry.mouse_enter = platform::env.ed->get_listeners<MouseEnterEvent>(event_id::mouse_enter);
        platform::event_registry.mouse_move_delta =
            platform::env.ed->get_listeners<PosEvent>(event_id::mouse_move_delta);
        platform::event_registry.mouse_move = platform::env.ed->get_listeners<PosEvent>(event_id::mouse_move);
        platform::event_registry.dpi_changed = platform::env.ed->get_listeners<DpiChangedEvent>(event_id::dpi_changed);
    }

    void init_library(acul::events::dispatcher *ed)
    {
        if (!platform::init_platform()) throw acul::runtime_error("Failed to initialize Window platform");
        platform::init_timer();
        set_time(0.0);
        platform::env.ed = ed;
        platform::env.default_cursor = Cursor::create(Cursor::Type::arrow);
    }

    void destroy_library()
    {
        LOG_INFO("Destroying Window platform");
        platform::env.default_cursor.reset();
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