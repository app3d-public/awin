#ifndef APP_WINDOW_POPUP_H
#define APP_WINDOW_POPUP_H

#include <acul/api.hpp>
#include <astl/vector.hpp>
#include <awin/window.hpp>
#include <initializer_list>

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
        APPLIB_API Buttons msgBox(const char *message, const char *title, Style style = Style::Info,
                                  std::initializer_list<Buttons> buttons = {Buttons::OK},
                                  awin::Window *parent = nullptr);

        // Displays a popup with "Yes" and "No" options, returning true if "Yes" is selected.
        inline bool confirmMsxBox(const char *message, const char *title, awin::Window *parent = nullptr)
        {
            return msgBox(message, title, Style::Question, {Buttons::Yes, Buttons::No}, parent) == Buttons::Yes;
        }

        struct FilePattern
        {
            std::string description;
            astl::vector<std::string> extensions;
        };

        APPLIB_API std::string openFileDialog(const char *title, const astl::vector<FilePattern> &pattern,
                                              const char *defaultPath, bool multiply);

        APPLIB_API std::string openFolderDialog(const char *title, const char *defaultPath);

        APPLIB_API std::string saveFileDialog(const char *title, const astl::vector<FilePattern> &pattern,
                                              const char *defaultPath);
    } // namespace popup
} // namespace awin

#endif