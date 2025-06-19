#pragma once

#include <awin/types.hpp>

namespace awin
{
    namespace platform
    {

        struct LinuxCursorCaller
        {
            platform::native_cursor_t *(*create)(Cursor::Type) = NULL;
            void (*assign)(struct LinuxWindowImpl *, platform::native_cursor_t *) = NULL;
            void (*destroy)(platform::native_cursor_t *) = NULL;
            bool (*valid)(const platform::native_cursor_t *) = NULL;
        };

        void init_call_cdata(LinuxCursorCaller &caller);
    } // namespace platform
} // namespace awin