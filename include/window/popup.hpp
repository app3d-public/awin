#ifndef APP_WINDOW_POPUP_H
#define APP_WINDOW_POPUP_H

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
            OKCancel,
            Quit,
            YesNo
        };

        // Represents the possible user responses in a popup window.
        enum class Selection
        {
            OK,
            Cancel,
            Yes,
            No,
            Quit,
            Error,
            None
        };

        // Displays a popup window with a message, title, and configurable style and buttons.
        Selection show(const char *message, const char *title, Style style = Style::Info,
                       Buttons buttons = Buttons::OK);

        // Displays a popup with "Yes" and "No" options, returning true if "Yes" is selected.
        inline bool confirm(const char *message, const char *title)
        {
            return show(message, title, Style::Question, Buttons::YesNo) == Selection::Yes;
        }
    } // namespace popup
} // namespace window

#endif