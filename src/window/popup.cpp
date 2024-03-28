#include <cassert>
#include <core/std/array.hpp>
#include <core/std/types@basic.hpp>
#include <sstream>
#include <tinyfiledialogs.h>
#include <window/popup.hpp>

#define BTN_OK     0x0
#define BTN_CANCEL 0x1
#define BTN_YES    0x2
#define BTN_NO     0x4

namespace window
{
    namespace popup
    {
        Buttons show(const char *message, const char *title, Style style, std::initializer_list<Buttons> buttons)
        {
            assert(buttons.size() > 0);
            const char *iconType;
            const char *buttonType;
            Array<Buttons> btn(buttons);
            std::sort(btn.begin(), btn.end());

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

            std::stringstream ss;
            u8 mask{0};
            for (auto button : buttons)
            {
                switch (button)
                {
                    case Buttons::OK:
                        ss << "ok";
                        mask |= BTN_OK;
                        break;
                    case Buttons::No:
                        ss << "no";
                        mask |= BTN_NO;
                        break;
                    case Buttons::Yes:
                        ss << "yes";
                        mask |= BTN_YES;
                        break;
                    case Buttons::Cancel:
                        ss << "cancel";
                        mask |= BTN_CANCEL;
                        break;
                    default:
                        break;
                }
            }
            buttonType = ss.str().c_str();

            int result = tinyfd_messageBox(title, message, buttonType, iconType, 1);

            switch (result)
            {
                case 0:
                    return mask & BTN_CANCEL ? Buttons::Cancel : Buttons::No;
                case 1:
                    return mask & BTN_YES ? Buttons::Yes : Buttons::OK;
                case 2:
                    return mask & BTN_NO ? Buttons::No : Buttons::Error;
                default:
                    return Buttons::Error;
            }
        }
    } // namespace popup
} // namespace window