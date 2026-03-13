#include <awin/awin.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "platform.hpp"

namespace awin::platform::x11
{
    namespace
    {
        inline void split_bpp(int bpp, VidMode::Color &color)
        {
            if (bpp == 32) bpp = 24;
            color.r = color.g = color.b = bpp / 3;
            const int delta = bpp - color.r * 3;
            if (delta >= 1) ++color.g;
            if (delta == 2) ++color.r;
        }

        inline int calculate_refresh_rate(const XRRModeInfo *mode)
        {
            if (!mode || !mode->hTotal || !mode->vTotal) return 0;
            return static_cast<int>(std::lround((double)mode->dotClock / ((double)mode->hTotal * (double)mode->vTotal)));
        }

        inline const XRRModeInfo *get_mode_info(const XRRScreenResources *sr, RRMode id)
        {
            if (!sr) return nullptr;
            for (int i = 0; i < sr->nmode; ++i)
                if (sr->modes[i].id == id) return sr->modes + i;
            return nullptr;
        }

        inline bool is_good_mode(const XRRModeInfo *mode)
        {
            if (!mode) return false;
            return (mode->modeFlags & RR_Interlace) == 0;
        }

        inline void fill_mode_from_randr(const XRRModeInfo *mode_info, const XRRCrtcInfo *crtc_info, VidMode &mode)
        {
            mode = {};
            if (crtc_info && (crtc_info->rotation == RR_Rotate_90 || crtc_info->rotation == RR_Rotate_270))
            {
                mode.width = static_cast<int>(mode_info->height);
                mode.height = static_cast<int>(mode_info->width);
            }
            else
            {
                mode.width = static_cast<int>(mode_info->width);
                mode.height = static_cast<int>(mode_info->height);
            }
            mode.refresh_rate = calculate_refresh_rate(mode_info);
            split_bpp(DefaultDepth(g_ctx->display, g_ctx->screen), mode.color_bits);
        }

        inline bool parse_output_id(const acul::string &system_name, RROutput &output)
        {
            output = None;
            if (system_name.empty()) return false;
            const char prefix[] = "x11-output:";
            if (system_name.find(prefix) != 0) return false;
            const char *value = system_name.c_str() + sizeof(prefix) - 1;
            char *end = nullptr;
            unsigned long parsed = strtoul(value, &end, 10);
            if (!end || *end != '\0') return false;
            output = static_cast<RROutput>(parsed);
            return true;
        }

        inline bool build_fallback_monitor(acul::vector<Monitor> &result)
        {
            Monitor monitor{};
            monitor.name = "X11 Display";
            monitor.system_name = "x11:0";
            monitor.position = {0, 0};
            monitor.dimensions = {DisplayWidth(g_ctx->display, g_ctx->screen), DisplayHeight(g_ctx->display, g_ctx->screen)};
            monitor.work_position = monitor.position;
            monitor.work_dimensions = monitor.dimensions;
            monitor.physical_size_mm = {DisplayWidthMM(g_ctx->display, g_ctx->screen),
                                        DisplayHeightMM(g_ctx->display, g_ctx->screen)};
            monitor.content_scale = g_ctx->dpi;
            if (monitor.content_scale.x <= 0.f) monitor.content_scale.x = 1.f;
            if (monitor.content_scale.y <= 0.f) monitor.content_scale.y = 1.f;
            monitor.primary = true;
            result.push_back(std::move(monitor));
            return true;
        }

        inline VidMode make_fallback_mode()
        {
            VidMode mode{};
            if (!g_ctx || !g_ctx->display) return mode;
            mode.width = DisplayWidth(g_ctx->display, g_ctx->screen);
            mode.height = DisplayHeight(g_ctx->display, g_ctx->screen);
            mode.refresh_rate = 60;
            split_bpp(DefaultDepth(g_ctx->display, g_ctx->screen), mode.color_bits);
            return mode;
        }
    } // namespace

    bool poll_monitors(acul::vector<Monitor> &result)
    {
        result.clear();
        if (!g_ctx || !g_ctx->display) return false;

        auto &randr = g_ctx->xlib.randr;
        if (!randr.init || !randr.XRRGetScreenResourcesCurrent || !randr.XRRGetOutputInfo || !randr.XRRGetCrtcInfo)
            return build_fallback_monitor(result);

        XRRScreenResources *sr = randr.XRRGetScreenResourcesCurrent(g_ctx->display, g_ctx->root);
        if (!sr) return build_fallback_monitor(result);

        const RROutput primary = randr.XRRGetOutputPrimary ? randr.XRRGetOutputPrimary(g_ctx->display, g_ctx->root) : None;
        bool primary_found = false;

        for (int i = 0; i < sr->noutput; ++i)
        {
            XRROutputInfo *oi = randr.XRRGetOutputInfo(g_ctx->display, sr, sr->outputs[i]);
            if (!oi) continue;
            if (oi->connection != RR_Connected || oi->crtc == None)
            {
                randr.XRRFreeOutputInfo(oi);
                continue;
            }

            XRRCrtcInfo *ci = randr.XRRGetCrtcInfo(g_ctx->display, sr, oi->crtc);
            if (!ci)
            {
                randr.XRRFreeOutputInfo(oi);
                continue;
            }

            Monitor monitor{};
            monitor.name = oi->name ? oi->name : "Display";
            monitor.system_name = acul::format("x11-output:%lu", static_cast<unsigned long>(sr->outputs[i]));
            monitor.position = {ci->x, ci->y};
            monitor.dimensions = {static_cast<i32>(ci->width), static_cast<i32>(ci->height)};
            monitor.work_position = monitor.position;
            monitor.work_dimensions = monitor.dimensions;

            int width_mm = oi->mm_width;
            int height_mm = oi->mm_height;
            if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270) std::swap(width_mm, height_mm);
            if (width_mm <= 0 || height_mm <= 0)
            {
                width_mm = static_cast<int>(ci->width * 25.4f / 96.f);
                height_mm = static_cast<int>(ci->height * 25.4f / 96.f);
            }
            monitor.physical_size_mm = {width_mm, height_mm};
            monitor.content_scale = g_ctx->dpi;
            if (monitor.content_scale.x <= 0.f) monitor.content_scale.x = 1.f;
            if (monitor.content_scale.y <= 0.f) monitor.content_scale.y = 1.f;

            monitor.primary = (primary != None && sr->outputs[i] == primary);
            primary_found = primary_found || monitor.primary;
            result.push_back(std::move(monitor));

            randr.XRRFreeCrtcInfo(ci);
            randr.XRRFreeOutputInfo(oi);
        }

        randr.XRRFreeScreenResources(sr);
        if (!result.empty() && !primary_found) result[0].primary = true;
        if (result.empty()) return build_fallback_monitor(result);
        return true;
    }

    bool get_monitor_video_mode(const Monitor *monitor, VidMode &result)
    {
        result = {};
        if (!monitor || !g_ctx || !g_ctx->display) return false;

        auto &randr = g_ctx->xlib.randr;
        if (!randr.init) 
        {
            result = make_fallback_mode();
            return result.width > 0 && result.height > 0;
        }

        RROutput output = None;
        if (!parse_output_id(monitor->system_name, output))
        {
            result = make_fallback_mode();
            return result.width > 0 && result.height > 0;
        }

        XRRScreenResources *sr = randr.XRRGetScreenResourcesCurrent(g_ctx->display, g_ctx->root);
        if (!sr) return false;

        XRROutputInfo *oi = randr.XRRGetOutputInfo(g_ctx->display, sr, output);
        if (!oi || oi->connection != RR_Connected || oi->crtc == None)
        {
            if (oi) randr.XRRFreeOutputInfo(oi);
            randr.XRRFreeScreenResources(sr);
            return false;
        }

        XRRCrtcInfo *ci = randr.XRRGetCrtcInfo(g_ctx->display, sr, oi->crtc);
        if (!ci)
        {
            randr.XRRFreeOutputInfo(oi);
            randr.XRRFreeScreenResources(sr);
            return false;
        }

        const XRRModeInfo *mode_info = get_mode_info(sr, ci->mode);
        if (mode_info && is_good_mode(mode_info)) fill_mode_from_randr(mode_info, ci, result);

        randr.XRRFreeCrtcInfo(ci);
        randr.XRRFreeOutputInfo(oi);
        randr.XRRFreeScreenResources(sr);
        return result.width > 0 && result.height > 0;
    }

    bool get_monitor_video_modes(const Monitor *monitor, acul::vector<VidMode> &result)
    {
        result.clear();
        if (!monitor || !g_ctx || !g_ctx->display) return false;

        auto &randr = g_ctx->xlib.randr;
        if (!randr.init)
        {
            VidMode fallback = make_fallback_mode();
            if (fallback.width <= 0 || fallback.height <= 0) return false;
            result.push_back(fallback);
            return true;
        }

        RROutput output = None;
        if (!parse_output_id(monitor->system_name, output)) return false;

        XRRScreenResources *sr = randr.XRRGetScreenResourcesCurrent(g_ctx->display, g_ctx->root);
        if (!sr) return false;

        XRROutputInfo *oi = randr.XRRGetOutputInfo(g_ctx->display, sr, output);
        if (!oi || oi->connection != RR_Connected || oi->crtc == None)
        {
            if (oi) randr.XRRFreeOutputInfo(oi);
            randr.XRRFreeScreenResources(sr);
            return false;
        }

        XRRCrtcInfo *ci = randr.XRRGetCrtcInfo(g_ctx->display, sr, oi->crtc);
        if (!ci)
        {
            randr.XRRFreeOutputInfo(oi);
            randr.XRRFreeScreenResources(sr);
            return false;
        }

        for (int i = 0; i < oi->nmode; ++i)
        {
            const XRRModeInfo *mode_info = get_mode_info(sr, oi->modes[i]);
            if (!is_good_mode(mode_info)) continue;
            VidMode mode{};
            fill_mode_from_randr(mode_info, ci, mode);
            if (mode.width > 0 && mode.height > 0) result.push_back(mode);
        }

        randr.XRRFreeCrtcInfo(ci);
        randr.XRRFreeOutputInfo(oi);
        randr.XRRFreeScreenResources(sr);

        if (result.empty())
        {
            VidMode current{};
            if (get_monitor_video_mode(monitor, current)) result.push_back(current);
        }
        return !result.empty();
    }
} // namespace awin::platform::x11
