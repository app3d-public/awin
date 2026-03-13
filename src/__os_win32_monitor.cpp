#include <acul/string/string.hpp>
#include <awin/awin.hpp>
#include <windows.h>

namespace awin::platform
{
    namespace
    {
        void split_bpp(int bpp, VidMode::Color &color)
        {
            if (bpp == 32) bpp = 24;
            color.r = color.g = color.b = bpp / 3;
            const int delta = bpp - color.r * 3;
            if (delta >= 1) ++color.g;
            if (delta == 2) ++color.r;
        }

        static inline bool utf8_to_utf16(const acul::string &src, acul::u16string &dst)
        {
            dst = acul::utf8_to_utf16(src);
            return !dst.empty();
        }

        static void fill_monitor_metrics(const WCHAR *device_name, Monitor &monitor)
        {
            DEVMODEW dm{};
            dm.dmSize = sizeof(dm);
            if (EnumDisplaySettingsExW(device_name, ENUM_CURRENT_SETTINGS, &dm, EDS_ROTATEDMODE))
            {
                monitor.position = {dm.dmPosition.x, dm.dmPosition.y};
                monitor.dimensions = {static_cast<i32>(dm.dmPelsWidth), static_cast<i32>(dm.dmPelsHeight)};
            }

            HMONITOR hmonitor = MonitorFromPoint(POINT{monitor.position.x, monitor.position.y}, MONITOR_DEFAULTTONULL);
            if (hmonitor)
            {
                MONITORINFOEXW info{};
                info.cbSize = sizeof(info);
                if (GetMonitorInfoW(hmonitor, &info))
                {
                    monitor.work_position = {info.rcWork.left, info.rcWork.top};
                    monitor.work_dimensions = {info.rcWork.right - info.rcWork.left,
                                               info.rcWork.bottom - info.rcWork.top};
                    monitor.primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
                }
            }

            HDC dc = CreateDCW(L"DISPLAY", device_name, NULL, NULL);
            if (dc)
            {
                monitor.physical_size_mm = {GetDeviceCaps(dc, HORZSIZE), GetDeviceCaps(dc, VERTSIZE)};
                const int dpi_x = GetDeviceCaps(dc, LOGPIXELSX);
                const int dpi_y = GetDeviceCaps(dc, LOGPIXELSY);
                monitor.content_scale.x = dpi_x > 0 ? dpi_x / 96.f : 1.f;
                monitor.content_scale.y = dpi_y > 0 ? dpi_y / 96.f : 1.f;
                DeleteDC(dc);
            }

            if (monitor.work_dimensions.x <= 0 || monitor.work_dimensions.y <= 0)
            {
                monitor.work_position = monitor.position;
                monitor.work_dimensions = monitor.dimensions;
            }
        }

        void append_monitor(DISPLAY_DEVICEW &adapter, DISPLAY_DEVICEW *display, acul::vector<Monitor> &result)
        {
            Monitor monitor{};
            acul::u16string w_name{(const c16 *)(display ? display->DeviceString : adapter.DeviceString)};
            monitor.name = acul::utf16_to_utf8(w_name);
            monitor.system_name = acul::utf16_to_utf8((const c16 *)(adapter.DeviceName));
            fill_monitor_metrics(adapter.DeviceName, monitor);
            result.push_back(std::move(monitor));
        }
    } // namespace

    bool poll_monitors(acul::vector<Monitor> &result)
    {
        result.clear();

        for (DWORD adapter_index = 0;; ++adapter_index)
        {
            DISPLAY_DEVICEW adapter{};
            adapter.cb = sizeof(adapter);
            if (!EnumDisplayDevicesW(NULL, adapter_index, &adapter, 0)) break;
            if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE)) continue;

            bool found_displays = false;
            for (DWORD display_index = 0;; ++display_index)
            {
                DISPLAY_DEVICEW display{};
                display.cb = sizeof(display);
                if (!EnumDisplayDevicesW(adapter.DeviceName, display_index, &display, 0)) break;
                if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE)) continue;
                found_displays = true;
                append_monitor(adapter, &display, result);
            }

            if (!found_displays) append_monitor(adapter, nullptr, result);
        }

        return true;
    }

    bool get_monitor_video_mode(const Monitor *monitor, VidMode &result)
    {
        result = {};
        if (!monitor || monitor->system_name.empty()) return false;

        acul::u16string device_name;
        if (!utf8_to_utf16(monitor->system_name, device_name)) return false;

        DEVMODEW dm{};
        dm.dmSize = sizeof(dm);
        if (!EnumDisplaySettingsW((LPCWSTR)device_name.c_str(), ENUM_CURRENT_SETTINGS, &dm)) return false;

        result.width = static_cast<int>(dm.dmPelsWidth);
        result.height = static_cast<int>(dm.dmPelsHeight);
        result.refresh_rate = static_cast<int>(dm.dmDisplayFrequency);
        split_bpp(static_cast<int>(dm.dmBitsPerPel), result.color_bits);
        return true;
    }

    bool get_monitor_video_modes(const Monitor *monitor, acul::vector<VidMode> &result)
    {
        result.clear();
        if (!monitor || monitor->system_name.empty()) return false;

        acul::u16string device_name;
        if (!utf8_to_utf16(monitor->system_name, device_name)) return false;

        for (DWORD mode_index = 0;; ++mode_index)
        {
            DEVMODEW dm{};
            dm.dmSize = sizeof(dm);
            if (!EnumDisplaySettingsW((LPCWSTR)device_name.c_str(), mode_index, &dm)) break;
            if (dm.dmBitsPerPel < 15) continue;

            VidMode mode{};
            mode.width = static_cast<int>(dm.dmPelsWidth);
            mode.height = static_cast<int>(dm.dmPelsHeight);
            mode.refresh_rate = static_cast<int>(dm.dmDisplayFrequency);
            split_bpp(static_cast<int>(dm.dmBitsPerPel), mode.color_bits);
            result.push_back(mode);
        }

        if (result.empty())
        {
            VidMode mode{};
            if (get_monitor_video_mode(monitor, mode)) result.push_back(mode);
        }

        return !result.empty();
    }
} // namespace awin::platform
