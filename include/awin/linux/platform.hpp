#pragma once

#include "x11/platform.hpp"
#include "x11/window.hpp"

namespace awin
{
    namespace platform
    {
        struct LinuxWindowData
        {
            x11::X11WindowData x11;
            bool raw_input{false};
        };

        using platform_data_t = LinuxWindowData;

        struct LinuxAccessConnect
        {
        };

        using native_access = LinuxAccessConnect;

        bool poll_posix(struct pollfd *fds, nfds_t count, f64 *timeout);
    } // namespace platform
} // namespace awin