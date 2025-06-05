#include <acul/log.hpp>
#include <awin/platform.hpp>
#include <awin/window.hpp>

#define WINDOW_BACKEND_UNKNOWN -1
#define WINDOW_BACKEND_X11     0
#define WINDOW_BACKEND_WAYLAND 1

namespace awin
{
    namespace platform
    {
        void init_timer()
        {
            env.timer.offset = CLOCK_REALTIME;
            env.timer.frequency = 1000000000;
#if defined(_POSIX_MONOTONIC_CLOCK)
            struct timespec ts;
            if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) env.timer.offset = CLOCK_MONOTONIC;
#endif
        };

        void destroy_platform() { x11::destroy_platform(); };

        int get_backend_type()
        {
            const char *xdg_session = getenv("XDG_SESSION_TYPE");
            if (xdg_session == nullptr)
                return WINDOW_BACKEND_UNKNOWN;
            else if (strcmp(xdg_session, "x11") == 0) { return WINDOW_BACKEND_X11; }
            else if (strcmp(xdg_session, "wayland") == 0)
                return WINDOW_BACKEND_WAYLAND;
            return WINDOW_BACKEND_UNKNOWN;
        }

        u64 get_time_frequency() { return env.timer.frequency; }

        bool init_platform()
        {
            switch (get_backend_type())
            {
                case WINDOW_BACKEND_X11:
                    return x11::init_platform();
                default:
                    LOG_ERROR("Unknown window backend");
                    return false;
            }
        }

        u64 get_time_value()
        {
            struct timespec ts;
            clock_gettime(env.timer.offset, &ts);
            return (u64)ts.tv_sec * env.timer.frequency + (u64)ts.tv_nsec;
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
    } // namespace platform

    Window::Window(const acul::string &title, i32 width, i32 height, CreationFlags flags)
    {
        platform::x11::create_window(&_platform, title, width, height, flags);
    }

    void poll_events() { platform::x11::poll_events(); }
} // namespace awin