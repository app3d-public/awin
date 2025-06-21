#pragma once

#ifdef _WIN32
    #include <windef.h>
#endif

namespace awin
{
    namespace platform
    {
        struct CursorPlatform
        {
#ifdef _WIN32
            HCURSOR cursor;

            explicit CursorPlatform(HCURSOR cursor = NULL) : cursor(cursor) {}

            bool valid() const { return cursor != NULL; }
#else
            bool valid() const;

            ~CursorPlatform();
#endif
        };
    } // namespace platform
} // namespace awin