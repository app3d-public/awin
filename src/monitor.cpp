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
        auto it = std::find_if(monitors.begin(), monitors.end(), [](const Monitor &m) { return m.primary; });
        return it != monitors.end() ? &*it : nullptr;
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
