#pragma once
#include <windef.h>

namespace window
{
    namespace platform
    {
        struct Win32Cursor
        {
            HCURSOR cursor;

            explicit Win32Cursor(HCURSOR cursor = NULL) : cursor(cursor) {}

            bool valid() const { return cursor != NULL; }
        };

        using native_cursor_t = Win32Cursor;
    } // namespace platform
} // namespace window