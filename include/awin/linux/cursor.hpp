#pragma once

namespace awin
{
    namespace platform
    {
        struct LinuxCursor
        {
            bool valid() const { return false; }
        };

        using native_cursor_t = LinuxCursor;
    } // namespace platform
} // namespace awin