#include <acul/log.hpp>
#include <acul/pair.hpp>
#include <awin/platform.hpp>
#include <awin/window.hpp>
#include "x11/platform.hpp"

namespace awin
{
    namespace platform
    {
        struct PlatformDataDispatcher
        {
            int backend_type;
            LinuxPlatformCaller pcall;
            LinuxWindowCaller wcall;
            LinuxCursorCaller ccall;
        } pd;

        void init_timer()
        {
            env.timer.clock_id = CLOCK_REALTIME;
            env.timer.frequency = 1000000000;
#if defined(_POSIX_MONOTONIC_CLOCK)
            struct timespec ts;
            if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
            {
                env.timer.clock_id = CLOCK_MONOTONIC;
                env.timer.offset = (u64)ts.tv_sec * 1'000'000'000 + ts.tv_nsec;
                return;
            }
#endif
            env.timer.offset = 0;
        };

        void destroy_platform()
        {
            if (pd.backend_type != WINDOW_BACKEND_UNKNOWN) pd.pcall.destroy_platform();
        };

        bool init_platform_caller()
        {
            pd.backend_type = WINDOW_BACKEND_UNKNOWN;
            const char *xdg_session = getenv("XDG_SESSION_TYPE");
            if (xdg_session == nullptr || strcmp(xdg_session, "wayland") == 0)
                return false;
            else if (strcmp(xdg_session, "x11") == 0)
            {
                platform::x11::init_pcall_data(pd.pcall);
                platform::x11::init_wcall_data(pd.wcall);
                platform::x11::init_ccall_data(pd.ccall);
                pd.backend_type = WINDOW_BACKEND_X11;
                return true;
            }
            return false;
        }

        u64 get_time_frequency() { return env.timer.frequency; }

        bool init_platform()
        {
            if (!init_platform_caller())
            {
                LOG_ERROR("Unknown window backend");
                return false;
            }
            return pd.pcall.init_platform();
        }

        u64 get_time_value()
        {
            struct timespec ts;
            if (clock_gettime(env.timer.clock_id, &ts) == 0)
                return (u64)ts.tv_sec * env.timer.frequency + (u64)ts.tv_nsec;
            return 0;
        }

        bool poll_posix(struct pollfd *fds, nfds_t count, f64 *timeout)
        {
            for (;;)
            {
                if (timeout)
                {
                    const u64 base = get_time_value();

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__CYGWIN__)
                    const time_t seconds = (time_t)*timeout;
                    const long nanoseconds = (long)((*timeout - seconds) * 1e9);
                    const struct timespec ts = {seconds, nanoseconds};
                    const int result = ppoll(fds, count, &ts, NULL);
#elif defined(__NetBSD__)
                    const time_t seconds = (time_t)*timeout;
                    const long nanoseconds = (long)((*timeout - seconds) * 1e9);
                    const struct timespec ts = {seconds, nanoseconds};
                    const int result = pollts(fds, count, &ts, NULL);
#else
                    const int milliseconds = (int)(*timeout * 1e3);
                    const int result = poll(fds, count, milliseconds);
#endif
                    const int error = errno; // clock_gettime may overwrite our error

                    *timeout -= (get_time_value() - base) / (f64)get_time_frequency();

                    if (result > 0)
                        return true;
                    else if (result == -1 && error != EINTR && error != EAGAIN)
                        return false;
                    else if (*timeout <= 0.0)
                        return false;
                }
                else
                {
                    const int result = poll(fds, count, -1);
                    if (result > 0)
                        return true;
                    else if (result == -1 && errno != EINTR && errno != EAGAIN)
                        return false;
                }
            }
        }

        CursorPlatform::~CursorPlatform()
        {
            if (pd.ccall.destroy) pd.ccall.destroy(this);
        }

        bool CursorPlatform::valid() const { return pd.ccall.valid(this); }
    } // namespace platform

    MonitorInfo get_primary_monitor_info() { return platform::pd.pcall.get_primary_monitor_info(); }

    Window::Window(const acul::string &title, i32 width, i32 height, WindowFlags flags)
    {
        _platform.backend = platform::pd.pcall.alloc_window_data();
        _platform.owner = this;
        if (!platform::pd.wcall.create_window(&_platform, title, width, height, flags))
            throw acul::runtime_error("Failed to create Window");
    }

    void Window::destroy() { platform::pd.wcall.destroy(_platform.backend); }

    void Window::show_window()
    {
        if (!hidden()) return;
        _platform.flags &= ~WindowFlagBits::Hidden;
        platform::pd.wcall.show_window(&_platform);
    }

    void Window::hide_window()
    {
        if (hidden()) return;
        platform::pd.wcall.hide_window(_platform.backend);
        _platform.flags |= WindowFlagBits::Hidden;
    }

    acul::string Window::title() const { return platform::pd.wcall.get_window_title(_platform.backend); }

    void Window::title(const acul::string &title) { platform::pd.wcall.set_window_title(_platform.backend, title); }

    void Window::enable_fullscreen()
    {
        _platform.flags |= WindowFlagBits::Fullscreen;
        platform::pd.wcall.enable_fullscreen(_platform.backend);
    }

    void Window::disable_fullscreen()
    {
        _platform.flags &= ~WindowFlagBits::Fullscreen;
        platform::pd.wcall.disable_fullscreen(_platform.backend);
    }

    acul::point2D<i32> Window::cursor_position() const
    {
        return platform::pd.wcall.get_cursor_position(_platform.backend);
    }

    void Window::cursor_position(acul::point2D<i32> position)
    {
        platform::pd.wcall.set_cursor_position(_platform.backend, position);
    }

    void Window::hide_cursor() { platform::pd.wcall.hide_cursor(&_platform); }

    void Window::show_cursor() { platform::pd.wcall.show_cursor(this, &_platform); }

    acul::point2D<i32> Window::position() const { return platform::pd.wcall.get_window_position(_platform.backend); }

    void Window::position(acul::point2D<i32> position) { platform::pd.wcall.set_window_position(&_platform, position); }

    void Window::center_window() { platform::pd.wcall.center_window(&_platform); }

    void Window::update_resize_limit() { platform::pd.wcall.update_resize_limit(&_platform); }

    void Window::minimize() { platform::pd.wcall.minimize_window(_platform.backend); }

    void Window::maximize() { platform::pd.wcall.maximize_window(&_platform); }

    void poll_events() { platform::pd.pcall.poll_events(); }

    void wait_events() { platform::pd.pcall.wait_events(); }

    void wait_events_timeout() { platform::pd.pcall.wait_events_timeout(); }

    void push_empty_event() { platform::pd.pcall.push_empty_event(); }

    f32 get_dpi() { return platform::pd.pcall.get_dpi(); }

    acul::point2D<i32> get_window_size(const Window &window)
    {
        return platform::pd.pcall.get_window_size(platform::native_access::get_window_data(window));
    }

    acul::string get_clipboard_string(const Window &window) { return platform::pd.pcall.get_clipboard_string(); }

    void set_clipboard_string(const Window &window, const acul::string &text)
    {
        platform::pd.pcall.set_clipboard_string(text);
    }

    void set_window_icon(Window &window, const acul::vector<Image> &images)
    {
        platform::pd.wcall.set_window_icon(window._platform.backend, images);
    }

    platform::LinuxWindowData *platform::native_access::get_window_data(const Window &window)
    {
        return window._platform.backend;
    }

    int platform::native_access::get_backend_type() { return pd.backend_type; }

    Cursor Cursor::create(Type type) { return {platform::pd.ccall.create(type)}; }

    void Cursor::assign(Window *window)
    {
        platform::pd.ccall.assign(platform::native_access::get_window_data(*window), _pd);
    }
} // namespace awin