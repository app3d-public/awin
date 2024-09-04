#ifndef APP_WINDOW_POPUP_H
#define APP_WINDOW_POPUP_H

#include <core/api.hpp>
#include <core/std/darray.hpp>
#include <initializer_list>


namespace window
{
    namespace popup
    {
        // Specifies the visual theme of popup windows to reflect the nature of the message.
        enum class Style
        {
            Info,
            Warning,
            Error,
            Question
        };

        // Defines the set of buttons to be displayed in a popup window.
        enum class Buttons
        {
            OK,
            Yes,
            No,
            Cancel,
            Error
        };

        // Displays a popup window with a message, title, and configurable style and buttons.
        APPLIB_API Buttons show(const char *message, const char *title, Style style = Style::Info,
                                std::initializer_list<Buttons> buttons = {Buttons::OK});

        // Displays a popup with "Yes" and "No" options, returning true if "Yes" is selected.
        inline bool confirm(const char *message, const char *title)
        {
            return show(message, title, Style::Question, {Buttons::Yes, Buttons::No}) == Buttons::Yes;
        }
    } // namespace popup
} // namespace window

#endif