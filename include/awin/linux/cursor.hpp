#pragma once

#include <cstddef>

namespace awin
{
    namespace platform
    {

        struct LinuxCursor
        {
            bool valid() const;
        };

        using native_cursor_t = LinuxCursor;
    } // namespace platform
} // namespace awin