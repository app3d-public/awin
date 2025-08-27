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
            
            acul::events::dispatch_event_group<KeyInputEvent>(event_registry.key_input, data->owner, key, action, mods);
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
        using namespace platform;

        assert(env.ed);
#ifdef _WIN32
        acul::events::cache_event_group(event_id::nc_mouse_down, event_registry.ncl_mouse_down, env.ed);
        acul::events::cache_event_group(event_id::nc_hit_test, event_registry.nc_hit_test, env.ed);
#endif
        acul::events::cache_event_group(event_id::focus, event_registry.focus, env.ed);
        acul::events::cache_event_group(event_id::scroll, event_registry.scroll, env.ed);
        acul::events::cache_event_group(event_id::minimize, event_registry.minimize, env.ed);
        acul::events::cache_event_group(event_id::maximize, event_registry.maximize, env.ed);
        acul::events::cache_event_group(event_id::resize, event_registry.resize, env.ed);
        acul::events::cache_event_group(event_id::move, event_registry.move, env.ed);
        acul::events::cache_event_group(event_id::char_input, event_registry.char_input, env.ed);
        acul::events::cache_event_group(event_id::key_input, event_registry.key_input, env.ed);
        acul::events::cache_event_group(event_id::mouse_click, event_registry.mouse_click, env.ed);
        acul::events::cache_event_group(event_id::mouse_enter, event_registry.mouse_enter, env.ed);
        acul::events::cache_event_group(event_id::mouse_move_delta, event_registry.mouse_move_delta, env.ed);
        acul::events::cache_event_group(event_id::mouse_move, event_registry.mouse_move, env.ed);
        acul::events::cache_event_group(event_id::dpi_changed, event_registry.dpi_changed, env.ed);
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