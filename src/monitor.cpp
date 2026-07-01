#include <acul/map.hpp>
#include <algorithm>
#include <awin/awin.hpp>
#include "env.hpp"

namespace awin
{
    namespace platform
    {
        bool poll_monitors(acul::vector<Monitor> &result);
        bool get_monitor_video_modes(const Monitor *monitor, acul::vector<VidMode> &result);
        bool get_monitor_video_mode(const Monitor *monitor, VidMode &result);
    } // namespace platform

    const acul::vector<Monitor> &get_monitors()
    {
        assert(platform::g_env);
        return platform::g_env->monitors;
    }

    const Monitor *get_primary_monitor()
    {
        const auto &monitors = get_monitors();
        if (platform::g_env->primary_monitor) return platform::g_env->primary_monitor;
        return monitors.empty() ? nullptr : &monitors[0];
    }

    namespace
    {
        struct Rect
        {
            i32 x;
            i32 y;
            i32 width;
            i32 height;
        };

        static i64 intersection_area(const Rect &lhs, const Rect &rhs)
        {
            const i32 left = std::max(lhs.x, rhs.x);
            const i32 top = std::max(lhs.y, rhs.y);
            const i32 right = std::min(lhs.x + lhs.width, rhs.x + rhs.width);
            const i32 bottom = std::min(lhs.y + lhs.height, rhs.y + rhs.height);
            if (right <= left || bottom <= top) return 0;
            return static_cast<i64>(right - left) * static_cast<i64>(bottom - top);
        }

        static Rect monitor_rect(const Monitor &monitor)
        {
            return {monitor.position.x, monitor.position.y, monitor.dimensions.x, monitor.dimensions.y};
        }

        static bool contains(const Rect &outer, const Rect &inner)
        {
            return inner.x >= outer.x && inner.y >= outer.y &&
                   inner.x + inner.width <= outer.x + outer.width &&
                   inner.y + inner.height <= outer.y + outer.height;
        }

        static Rect window_rect(const WindowData *window)
        {
            const auto pos = window->owner->position();
            return {pos.x, pos.y, window->dimenstions.x, window->dimenstions.y};
        }
    } // namespace

    const Monitor *get_window_monitor(const Window &window)
    {
        auto *data = get_window_data(window);
        return data && data->active_monitor ? data->active_monitor : get_primary_monitor();
    }

    const Monitor *find_window_monitor_by_overlap(const WindowData *window)
    {
        if (!window || !window->owner) return get_primary_monitor();

        const Rect rect = window_rect(window);
        const Monitor *best = nullptr;
        i64 best_area = 0;
        for (const auto &monitor : get_monitors())
        {
            const i64 area = intersection_area(rect, monitor_rect(monitor));
            if (area > best_area)
            {
                best = &monitor;
                best_area = area;
            }
        }
        return best ? best : get_primary_monitor();
    }

    bool update_window_monitor_by_overlap(WindowData *window, bool notify)
    {
        if (!window || !window->owner) return false;
        if (window->active_monitor && contains(monitor_rect(*window->active_monitor), window_rect(window)))
            return false;
        return update_window_monitor(window, find_window_monitor_by_overlap(window), notify);
    }

    bool update_window_monitor(WindowData *window, const Monitor *monitor, bool notify)
    {
        if (!window) return false;
        if ((!window->active_monitor && !monitor) ||
            (window->active_monitor && monitor && window->active_monitor->position == monitor->position))
        {
            window->active_monitor = monitor;
            return false;
        }

        window->active_monitor = monitor;
        if (notify && platform::g_env->events.monitor_change)
            acul::events::dispatch_event_group<MonitorChangeEvent>(
                platform::g_env->events.monitor_change, window->owner, monitor);
        return true;
    }

    acul::vector<VidMode> get_monitor_video_modes(const Monitor *monitor)
    {
        acul::vector<VidMode> raw_modes;
        if (!monitor || !platform::get_monitor_video_modes(monitor, raw_modes)) return {};

        acul::map<VidMode, bool> unique_modes;
        for (const auto &mode : raw_modes) unique_modes.emplace(mode, true);

        acul::vector<VidMode> result;
        result.reserve(unique_modes.size());
        for (const auto &[mode, _] : unique_modes) result.push_back(mode);

        return result;
    }

    VidMode get_monitor_video_mode(const Monitor *monitor)
    {
        VidMode result{};
        if (!monitor) return result;
        platform::get_monitor_video_mode(monitor, result);
        return result;
    }

} // namespace awin
