#ifndef APP_WINDOW_POPUP_H
#define APP_WINDOW_POPUP_H

#include <acul/api.hpp>
#include <acul/string/string.hpp>
#include <acul/vector.hpp>
#include <initializer_list>
#include "window.hpp"

namespace awin
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
        APPLIB_API Buttons message_box(const char *message, const char *title, Style style = Style::Info,
                                       std::initializer_list<Buttons> buttons = {Buttons::OK},
                                       awin::Window *parent = nullptr);

        // Displays a popup with "Yes" and "No" options, returning true if "Yes" is selected.
        inline bool confirm_message_box(const char *message, const char *title, awin::Window *parent = nullptr)
        {
            return message_box(message, title, Style::Question, {Buttons::Yes, Buttons::No}, parent) == Buttons::Yes;
        }

        struct FilePattern
        {
            acul::string description;
            acul::vector<acul::string> extensions;
        };

        APPLIB_API acul::string open_file_dialog(const char *title, const acul::vector<FilePattern> &pattern,
                                                 const char *default_path = "");

        APPLIB_API acul::vector<acul::string> open_file_dialog_multiple(const char *title,
                                                                        const acul::vector<FilePattern> &pattern,
                                                                        const char *default_path = "");

        APPLIB_API acul::string open_folder_dialog(const char *title, const char *defaultPath = "");

        APPLIB_API acul::string save_file_dialog(const char *title, const acul::vector<FilePattern> &pattern,
                                                 const char *default_path = "");
    } // namespace popup
} // namespace awin

#endif