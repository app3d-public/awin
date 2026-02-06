#include <acul/string/sstream.hpp>
#include <acul/string/string.hpp>
#include <awin/popup.hpp>

namespace awin::popup
{
    bool check_app_exists(const char *app)
    {
        const char *path_env = getenv("PATH");
        if (!path_env || !*app) return false;

        acul::string_view path(path_env);
        size_t start = 0;

        while (start < path.size())
        {
            size_t end = path.find(':', start);
            if (end == acul::string::npos) end = path.size();

            acul::string dir(path.substr(start, end - start));
            if (!dir.empty() && dir.back() != '/') dir += '/';

            acul::string full_path = dir + app;
            if (::access(full_path.c_str(), X_OK) == 0) return true;

            start = end + 1;
        }

        return false;
    }

    enum BackendType
    {
        undefined,
        kdialog,
        zenity
    };

    struct Context
    {
        BackendType backend;
        bool init;
    } ctx{BackendType::undefined, false};

    void init_ctx()
    {
        if (check_app_exists("kdialog"))
            ctx.backend = BackendType::kdialog;
        else if (check_app_exists("zenity"))
            ctx.backend = BackendType::zenity;
        ctx.init = true;
    }

    Buttons message_box(const char *message, const char *title, Style style, std::initializer_list<Buttons> buttons,
                        awin::Window *)
    {
        if (!ctx.init) init_ctx();
        acul::string icon;
        switch (style)
        {
            case Style::info:
                icon = "information";
                break;
            case Style::warning:
                icon = "warning";
                break;
            case Style::error:
                icon = "error";
                break;
            case Style::question:
                icon = "question";
                break;
        }

        acul::stringstream cmd;
        switch (ctx.backend)
        {
            case kdialog:
                cmd << "kdialog --";
                if (buttons.size() == 2 && *buttons.begin() == Buttons::yes && *(buttons.begin() + 1) == Buttons::no)
                    cmd << "yesno ";
                else if (buttons.size() == 2 && *buttons.begin() == Buttons::ok &&
                         *(buttons.begin() + 1) == Buttons::cancel)
                    cmd << "yesno --yes-label 'OK' --no-label 'Cancel' ";
                else
                    cmd << "msgbox ";
                cmd << "\"" << message << "\"";
                if (title && *title) cmd << " --title \"" << title << "\"";
                break;
            case zenity:
                cmd << "zenity --";
                if (buttons.size() == 2 && *buttons.begin() == Buttons::yes && *(buttons.begin() + 1) == Buttons::no)
                    cmd << "question";
                else if (buttons.size() == 2 && *buttons.begin() == Buttons::ok &&
                         *(buttons.begin() + 1) == Buttons::cancel)
                    cmd << "question --ok-label='OK' --cancel-label='Cancel'";
                else if (style == Style::error)
                    cmd << "error";
                else if (style == Style::warning)
                    cmd << "warning";
                else
                    cmd << "info";
                cmd << " --icon-name=dialog-" << icon;
                if (title && *title) cmd << " --title=\"" << title << "\"";
                if (message && *message) cmd << " --text=\"" << message << "\"";
                break;
            default:
                // fallback to stderr
                fprintf(stderr, "%s: %s\n", title ? title : "Message", message ? message : "");
                return Buttons::error;
        }
        cmd << " 2>/dev/null"; // suppress GTK or shell warnings

        FILE *pipe = popen(cmd.str().c_str(), "r");
        if (!pipe) return Buttons::error;

        char buffer[256] = {};
        fgets(buffer, sizeof(buffer), pipe);
        int code = pclose(pipe);

        // Interpret result
        switch (ctx.backend)
        {
            case kdialog:
                return code == 0 ? Buttons::yes : Buttons::no;

            case zenity:
                if (code == 0) // OK or Yes
                {
                    if (buttons.size() == 2) return *buttons.begin();
                    return Buttons::ok;
                }
                else // Cancel or No
                {
                    if (buttons.size() == 2) return *(buttons.begin() + 1);
                    return Buttons::cancel;
                }

            default:
                return Buttons::error;
        }
    }

    acul::string open_file_dialog(const char *title, const acul::vector<FilePattern> &pattern, const char *default_path)
    {
        if (!ctx.init) init_ctx();
        acul::stringstream cmd;

        switch (ctx.backend)
        {
            case kdialog:
                cmd << "kdialog --getopenfilename";
                if (default_path && *default_path)
                    cmd << " \"" << default_path << "\"";
                else
                    cmd << " .";

                if (!pattern.empty())
                {
                    cmd << " \"";
                    for (size_t i = 0; i < pattern.size(); ++i)
                    {
                        for (size_t j = 0; j < pattern[i].extensions.size(); ++j)
                        {
                            cmd << "*." << pattern[i].extensions[j];
                            if (j + 1 < pattern[i].extensions.size()) cmd << ' ';
                        }
                        if (i + 1 < pattern.size()) cmd << ' ';
                    }
                    cmd << "\"";
                }

                if (title && *title) cmd << " --title \"" << title << "\"";
                break;

            case zenity:
                cmd << "zenity --file-selection";
                if (default_path && *default_path) cmd << " --filename=\"" << default_path << "/\"";

                if (title && *title) cmd << " --title=\"" << title << "\"";

                for (const auto &p : pattern)
                {
                    cmd << " --file-filter=\"" << p.description << " |";
                    for (const auto &ext : p.extensions) cmd << " *." << ext;
                    cmd << "\"";
                }
                break;

            default:
                return "";
        }

        cmd << " 2>/dev/null";

        FILE *pipe = popen(cmd.str().c_str(), "r");
        if (!pipe) return "";

        char buffer[PATH_MAX] = {};
        fgets(buffer, sizeof(buffer), pipe);
        pclose(pipe);

        acul::string raw((char *)buffer);
        if (!raw.empty() && raw.back() == '\n') raw = raw.substr(0, raw.size() - 1);

        return raw;
    }

    acul::vector<acul::string> open_file_dialog_multiple(const char *title, const acul::vector<FilePattern> &pattern,
                                                         const char *default_path)
    {
        if (!ctx.init) init_ctx();

        acul::stringstream cmd;

        switch (ctx.backend)
        {
            case kdialog:
                cmd << "kdialog --getopenfilename";
                if (default_path && *default_path)
                    cmd << " \"" << default_path << "\"";
                else
                    cmd << " .";

                if (!pattern.empty())
                {
                    cmd << " \"";
                    for (size_t i = 0; i < pattern.size(); ++i)
                    {
                        for (size_t j = 0; j < pattern[i].extensions.size(); ++j)
                        {
                            cmd << "*." << pattern[i].extensions[j];
                            if (j + 1 < pattern[i].extensions.size()) cmd << ' ';
                        }
                        if (i + 1 < pattern.size()) cmd << ' ';
                    }
                    cmd << "\"";
                }

                if (title && *title) cmd << " --title \"" << title << "\"";

                cmd << " --multiple --separate-output";
                break;

            case zenity:
                cmd << "zenity --file-selection";
                if (default_path && *default_path) cmd << " --filename=\"" << default_path << "/\"";

                cmd << " --multiple --separator='|'";

                if (title && *title) cmd << " --title=\"" << title << "\"";

                for (const auto &p : pattern)
                {
                    cmd << " --file-filter=\"" << p.description << " |";
                    for (const auto &ext : p.extensions) cmd << " *." << ext;
                    cmd << "\"";
                }
                break;

            default:
                return {};
        }

        cmd << " 2>/dev/null";

        FILE *pipe = popen(cmd.str().c_str(), "r");
        if (!pipe) return {};

        char buffer[8192] = {};
        fgets(buffer, sizeof(buffer), pipe);
        pclose(pipe);

        acul::string raw(buffer);
        if (!raw.empty() && raw.back() == '\n') raw = raw.substr(0, raw.size() - 1);

        acul::vector<acul::string> result;

        if (ctx.backend == kdialog)
        {
            size_t start = 0;
            while (start < raw.size())
            {
                size_t end = raw.find('\n', start);
                if (end == acul::string::npos) end = raw.size();
                if (end > start) result.emplace_back(raw.substr(start, end - start));
                start = end + 1;
            }
        }
        else if (ctx.backend == zenity)
        {
            size_t start = 0;
            while (start < raw.size())
            {
                size_t sep = raw.find('|', start);
                if (sep == acul::string::npos)
                {
                    result.emplace_back(raw.substr(start));
                    break;
                }
                result.emplace_back(raw.substr(start, sep - start));
                start = sep + 1;
            }
        }

        return result;
    }

    acul::string open_folder_dialog(const char *title, const char *default_path)
    {
        if (!ctx.init) init_ctx();

        acul::stringstream cmd;
        switch (ctx.backend)
        {
            case kdialog:
                cmd << "kdialog --getexistingdirectory";
                if (default_path && *default_path)
                    cmd << " \"" << default_path << "\"";
                else
                    cmd << " .";

                if (title && *title) cmd << " --title \"" << title << "\"";
                break;

            case zenity:
                cmd << "zenity --file-selection --directory";
                if (default_path && *default_path) cmd << " --filename=\"" << default_path << "/\"";

                if (title && *title) cmd << " --title=\"" << title << "\"";
                break;

            default:
                return "";
        }

        cmd << " 2>/dev/null";

        FILE *pipe = popen(cmd.str().c_str(), "r");
        if (!pipe) return "";

        char buffer[4096] = {};
        fgets(buffer, sizeof(buffer), pipe);
        pclose(pipe);

        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        return acul::string((char *)buffer);
    }

    acul::string save_file_dialog(const char *title, const acul::vector<FilePattern> &pattern, const char *default_path)
    {
        if (!ctx.init) init_ctx();

        acul::stringstream cmd;
        switch (ctx.backend)
        {
            case kdialog:
                cmd << "kdialog --getsavefilename";
                if (default_path && *default_path)
                    cmd << " \"" << default_path << "\"";
                else
                    cmd << " .";

                if (!pattern.empty())
                {
                    cmd << " \"";
                    for (size_t i = 0; i < pattern.size(); ++i)
                    {
                        for (size_t j = 0; j < pattern[i].extensions.size(); ++j)
                        {
                            cmd << "*." << pattern[i].extensions[j];
                            if (j + 1 < pattern[i].extensions.size()) cmd << ' ';
                        }
                        if (i + 1 < pattern.size()) cmd << ' ';
                    }
                    cmd << "\"";
                }

                if (title && *title) cmd << " --title \"" << title << "\"";
                break;

            case zenity:
                cmd << "zenity --file-selection --save --confirm-overwrite";
                if (default_path && *default_path) cmd << " --filename=\"" << default_path << "\"";

                if (title && *title) cmd << " --title=\"" << title << "\"";

                for (const auto &pat : pattern)
                {
                    cmd << " --file-filter=\"";
                    cmd << pat.description << " | ";
                    for (size_t i = 0; i < pat.extensions.size(); ++i)
                    {
                        cmd << "*." << pat.extensions[i];
                        if (i + 1 < pat.extensions.size()) cmd << ' ';
                    }
                    cmd << "\"";
                }
                break;

            default:
                return "";
        }

        cmd << " 2>/dev/null";

        FILE *pipe = popen(cmd.str().c_str(), "r");
        if (!pipe) return "";

        char buffer[PATH_MAX] = {};
        fgets(buffer, sizeof(buffer), pipe);
        pclose(pipe);

        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';

        return acul::string((char *)buffer);
    }

} // namespace awin::popup