#include <window/popup.hpp>
#include <cstring>
#include <tinyfiledialogs.h>

namespace window
{
    namespace popup
    {
        Selection show(const char *message, const char *title, Style style, Buttons buttons)
        {
            const char *iconType;
            const char *buttonType;

            switch (style)
            {
                case Style::Info:
                    iconType = "info";
                    break;
                case Style::Warning:
                    iconType = "warning";
                    break;
                case Style::Error:
                    iconType = "error";
                    break;
                case Style::Question:
                    iconType = "question";
                    break;
            }

            // Определение типа кнопок
            switch (buttons)
            {
                case Buttons::OK:
                    buttonType = "ok";
                    break;
                case Buttons::OKCancel:
                    buttonType = "okcancel";
                    break;
                case Buttons::Quit:
                    buttonType = "quit";
                    break;
                case Buttons::YesNo:
                    buttonType = "yesno";
                    break;
            }

            // Обработка результата
            int result = tinyfd_messageBox(title, message, buttonType, iconType, 1);

            // Обработка результата
            if (strcmp(buttonType, "ok") == 0)
            {
                switch (result)
                {
                    case 1:
                        return Selection::OK;
                    case 0:
                        return Selection::Cancel;
                    default:
                        return Selection::Error;
                }
            }
            else if (strcmp(buttonType, "okcancel") == 0)
            {
                switch (result)
                {
                    case 1:
                        return Selection::OK;
                    case 0:
                        return Selection::Cancel;
                    default:
                        return Selection::Error;
                }
            }
            else if (strcmp(buttonType, "yesno") == 0)
            {
                switch (result)
                {
                    case 1:
                        return Selection::Yes;
                    case 0:
                        return Selection::No;
                    default:
                        return Selection::Error;
                }
            }
            else if (strcmp(buttonType, "yesnocancel") == 0)
            {
                switch (result)
                {
                    case 1:
                        return Selection::Yes;
                    case 2:
                        return Selection::No;
                    case 0:
                        return Selection::Cancel;
                    default:
                        return Selection::Error;
                }
            }
            else
            {
                return Selection::Error; // Возвращаем ошибку для любых других случаев
            }
        }
    } // namespace popup
} // namespace window