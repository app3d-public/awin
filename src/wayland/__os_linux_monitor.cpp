#include <awin/awin.hpp>
#include <cstdlib>
#include "platform.hpp"

namespace awin::platform::wayland
{
    namespace
    {
        static bool parse_output_id(const acul::string &system_name, u32 &output_id)
        {
            if (system_name.empty()) return false;
            const char prefix[] = "wl_output:";
            if (system_name.find(prefix) != 0) return false;
            const char *value = system_name.c_str() + sizeof(prefix) - 1;
            char *end = nullptr;
            unsigned long parsed = strtoul(value, &end, 10);
            if (!end || *end != '\0') return false;
            output_id = static_cast<u32>(parsed);
            return true;
        }

        static Output *find_output(const Monitor *monitor)
        {
            if (!monitor || !g_ctx) return nullptr;
            u32 output_id = 0;
            if (!parse_output_id(monitor->system_name, output_id)) return nullptr;
            for (auto &output : g_ctx->outputs)
                if (output.name_id == output_id) return &output;
            return nullptr;
        }

        static bool fill_current_mode(const Output &output, VidMode &mode)
        {
            mode = {};
            if (output.dimensions.x <= 0 || output.dimensions.y <= 0) return false;
            mode.width = output.dimensions.x;
            mode.height = output.dimensions.y;
            mode.color_bits = {8, 8, 8};
            mode.refresh_rate = output.current_mode > 0 ? (output.current_mode + 500) / 1000 : 60;
            return true;
        }
    } // namespace

    bool poll_monitors(acul::vector<Monitor> &result)
    {
        result.clear();
        g_env->primary_monitor = nullptr;
        if (!g_ctx) return false;

        result.reserve(g_ctx->outputs.size());
        for (const auto &output : g_ctx->outputs)
        {
            Monitor monitor{};
            monitor.name =
                output.name.empty() ? acul::format("Wayland Output %u", static_cast<unsigned>(output.name_id))
                                    : output.name;
            monitor.system_name = acul::format("wl_output:%u", static_cast<unsigned>(output.name_id));
            monitor.position = output.pos;
            monitor.work_position = output.pos;
            monitor.dimensions = output.dimensions;
            monitor.work_dimensions = output.dimensions;
            monitor.physical_size_mm = output.physical_size;
            monitor.content_scale.x = output.scale > 0 ? static_cast<f32>(output.scale) : 1.f;
            monitor.content_scale.y = monitor.content_scale.x;
            result.push_back(std::move(monitor));
        }
        if (!result.empty()) g_env->primary_monitor = &result[0];
        return !result.empty();
    }

    bool get_monitor_video_mode(const Monitor *monitor, VidMode &result)
    {
        result = {};
        auto *output = find_output(monitor);
        if (!output) return false;
        return fill_current_mode(*output, result);
    }

    bool get_monitor_video_modes(const Monitor *monitor, acul::vector<VidMode> &result)
    {
        result.clear();
        VidMode current{};
        if (!get_monitor_video_mode(monitor, current)) return false;
        result.push_back(current);
        return true;
    }
} // namespace awin::platform::wayland
