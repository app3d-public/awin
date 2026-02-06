#include <acul/log.hpp>
#include <awin/window.hpp>
#include "env.hpp"

namespace awin
{
    namespace platform
    {
        WindowEnvironment *g_env{nullptr};

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

            acul::events::dispatch_event_group<KeyInputEvent>(g_env->events.key_input, data->owner, key, action, mods);
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

        assert(g_env && g_env->ed);
        auto *ed = g_env->ed;
        auto &events = g_env->events;
#ifdef _WIN32
        acul::events::cache_event_group(event_id::nc_mouse_down, events.ncl_mouse_down, ed);
        acul::events::cache_event_group(event_id::nc_hit_test, events.nc_hit_test, ed);
#endif
        acul::events::cache_event_group(event_id::focus, events.focus, ed);
        acul::events::cache_event_group(event_id::scroll, events.scroll, ed);
        acul::events::cache_event_group(event_id::minimize, events.minimize, ed);
        acul::events::cache_event_group(event_id::maximize, events.maximize, ed);
        acul::events::cache_event_group(event_id::resize, events.resize, ed);
        acul::events::cache_event_group(event_id::move, events.move, ed);
        acul::events::cache_event_group(event_id::char_input, events.char_input, ed);
        acul::events::cache_event_group(event_id::key_input, events.key_input, ed);
        acul::events::cache_event_group(event_id::mouse_click, events.mouse_click, ed);
        acul::events::cache_event_group(event_id::mouse_enter, events.mouse_enter, ed);
        acul::events::cache_event_group(event_id::mouse_move_delta, events.mouse_move_delta, ed);
        acul::events::cache_event_group(event_id::mouse_move, events.mouse_move, ed);
        acul::events::cache_event_group(event_id::dpi_changed, events.dpi_changed, ed);
    }

    void init_library(const InitConfig &config)
    {
        platform::g_env = acul::alloc<platform::WindowEnvironment>();
        platform::g_env->log_service = config.log_service;
        platform::g_env->logger = config.logger;
        if (!platform::init_platform()) throw acul::runtime_error("Failed to initialize Window platform");
        platform::init_timer();
        set_time(0.0);
        platform::g_env->ed = config.events_dispatcher;
        platform::g_env->default_cursor = Cursor::create(Cursor::Type::arrow);
    }

    void destroy_library()
    {
        assert(platform::g_env);
        AWIN_LOG_INFO("Destroying Window library");
        platform::g_env->default_cursor.reset();
        platform::destroy_platform();
        acul::release(platform::g_env);
        platform::g_env = nullptr;
    }

    f64 get_time()
    {
        return static_cast<f64>(platform::get_time_value() - platform::g_env->timer.offset) /
               platform::get_time_frequency();
    }

    void set_time(f64 time)
    {
        if (std::isnan(time) || time < 0.0 || time > 18446744073.0)
        {
            AWIN_LOG_ERROR("Invalid time value: %f", time);
            return;
        }
        platform::g_env->timer.offset =
            platform::get_time_value() - static_cast<u64>(time * platform::get_time_frequency());
    }
} // namespace awin