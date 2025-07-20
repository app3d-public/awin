#include <acul/io/path.hpp>
#include <acul/string/string.hpp>
#include <awin/popup.hpp>
#include <windows.h>
// Include windows first
#include <acul/string/utils.hpp>
#include <awin/native_access.hpp>
#include <commdlg.h>
#include <shlobj.h>
#include <wchar.h>

#define BTN_OK     0x1
#define BTN_CANCEL 0x2
#define BTN_YES    0x4
#define BTN_NO     0x8

namespace awin
{
    namespace popup
    {
        inline int style_to_icon(Style style)
        {
            switch (style)
            {
                case Style::info:
                    return MB_ICONINFORMATION;
                case Style::warning:
                    return MB_ICONWARNING;
                case Style::error:
                    return MB_ICONERROR;
                case Style::question:
                    return MB_ICONQUESTION;
            }
            return MB_ICONINFORMATION;
        }

        int buttons_to_flags(std::initializer_list<Buttons> buttons)
        {
            int mask = 0x0;
            for (auto btn : buttons)
            {
                switch (btn)
                {
                    case Buttons::ok:
                        mask |= BTN_OK;
                        break;
                    case Buttons::yes:
                        mask |= BTN_YES;
                        break;
                    case Buttons::no:
                        mask |= BTN_NO;
                        break;
                    case Buttons::cancel:
                        mask |= BTN_CANCEL;
                        break;
                    default:
                        break;
                };
            }
            if (mask & BTN_YES && mask & BTN_NO && mask & BTN_CANCEL) return MB_YESNOCANCEL;
            if (mask & BTN_YES && mask & BTN_NO) return MB_YESNO;
            if (mask & BTN_OK && mask & BTN_CANCEL) return MB_OKCANCEL;
            return MB_OK;
        }

        Buttons message_box(const char *message, const char *title, Style style, std::initializer_list<Buttons> buttons,
                            awin::Window *parent)
        {
            UINT icon_flag = style_to_icon(style);
            UINT buttons_flag = buttons_to_flags(buttons);
            UINT flags = buttons_flag | icon_flag;
            HWND hwnd = parent ? native_access::get_hwnd(*parent) : nullptr;
            if (!hwnd) flags |= MB_TOPMOST;
            auto w_message = acul::utf8_to_utf16(message);
            LPCWSTR lp_text = reinterpret_cast<LPCWSTR>(w_message.c_str());
            auto w_title = acul::utf8_to_utf16(title);
            LPCWSTR lp_caption = reinterpret_cast<LPCWSTR>(w_title.c_str());
            switch (MessageBoxW(hwnd, lp_text, lp_caption, flags))
            {
                case IDOK:
                    return Buttons::ok;
                case IDYES:
                    return Buttons::yes;
                case IDNO:
                    return Buttons::no;
                case IDCANCEL:
                    return Buttons::cancel;
                default:
                    return Buttons::error;
            };
        }

        void fill_ofn_pattern(const acul::vector<FilePattern> &src, acul::u16string &dst)
        {
            for (const auto &pat : src)
            {
                dst += acul::utf8_to_utf16(pat.description) + u'\0';
                for (size_t i = 0; i < pat.extensions.size(); ++i)
                {
                    dst += acul::utf8_to_utf16(pat.extensions[i]);
                    if (i < pat.extensions.size() - 1) dst += u';';
                }
                dst += u'\0';
            }
            dst += u'\0';
        }

        OPENFILENAMEW get_ofn(const char *title, const char *default_path, const acul::u16string &pattern,
                              wchar_t *buffer)
        {
            acul::u16string w_title = acul::utf8_to_utf16(title);
            acul::u16string w_default_path = default_path ? acul::utf8_to_utf16(default_path) : u"";

            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = GetForegroundWindow();
            ofn.lpstrFilter = reinterpret_cast<LPCWSTR>(pattern.c_str());
            ofn.lpstrFile = buffer;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = reinterpret_cast<LPCWSTR>(w_title.c_str());
            ofn.lpstrInitialDir = w_default_path.empty() ? NULL : reinterpret_cast<LPCWSTR>(w_default_path.c_str());
            ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            return ofn;
        }

        acul::string open_file_dialog(const char *title, const acul::vector<FilePattern> &pattern,
                                      const char *default_path)
        {
            acul::u16string filter_patterns;
            fill_ofn_pattern(pattern, filter_patterns);
            wchar_t filename[MAX_PATH] = {0};
            OPENFILENAMEW ofn = get_ofn(title, default_path, filter_patterns, filename);
            if (GetOpenFileNameW(&ofn) == 0) return "";
            return acul::utf16_to_utf8(reinterpret_cast<const acul::u16string::value_type *>(filename));
        }

        acul::vector<acul::string>
        open_file_dialog_multiple(const char *title, const acul::vector<FilePattern> &pattern, const char *default_path)
        {
            acul::u16string filter_patterns;
            fill_ofn_pattern(pattern, filter_patterns);
            wchar_t buffer[8192] = {0};
            OPENFILENAMEW ofn = get_ofn(title, default_path, filter_patterns, buffer);
            ofn.Flags |= OFN_ALLOWMULTISELECT;
            if (GetOpenFileNameW(&ofn) == 0) return {};

            static_assert(sizeof(wchar_t) == sizeof(char16_t), "Incompatible wchar_t and char16_t");
            const c16 *ptr = (c16 *)buffer;
            acul::u16string dir = ptr;
            ptr += dir.size() + 1;

            acul::vector<acul::string> result;

            if (*ptr == L'\0')
                result.emplace_back(acul::utf16_to_utf8(dir));
            else
            {
                while (*ptr)
                {
                    acul::u16string full = dir + u'\\' + ptr;
                    result.emplace_back(acul::utf16_to_utf8(full));
                    ptr += null_terminated_length(ptr) + 1;
                }
            }
            return result;
        }

        acul::string open_folder_dialog(const char *title, const char *default_path)
        {
            IFileOpenDialog *file_open_dlg = nullptr;

            HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                                          reinterpret_cast<void **>(&file_open_dlg));
            if (FAILED(hr))
            {
                CoUninitialize();
                return "";
            }

            DWORD dw_options;
            hr = file_open_dlg->GetOptions(&dw_options);
            if (SUCCEEDED(hr)) file_open_dlg->SetOptions(dw_options | FOS_PICKFOLDERS);

            if (title)
            {
                acul::u16string w_title = acul::utf8_to_utf16(title);
                file_open_dlg->SetTitle(reinterpret_cast<LPCWSTR>(w_title.c_str()));
            }

            if (default_path)
            {
                acul::u16string w_default_path = acul::utf8_to_utf16(default_path);
                IShellItem *pItem = nullptr;
                hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(w_default_path.c_str()), NULL,
                                                 IID_PPV_ARGS(&pItem));
                if (SUCCEEDED(hr))
                {
                    file_open_dlg->SetFolder(pItem);
                    pItem->Release();
                }
            }

            hr = file_open_dlg->Show(NULL);
            if (SUCCEEDED(hr))
            {
                IShellItem *item = nullptr;
                hr = file_open_dlg->GetResult(&item);
                if (SUCCEEDED(hr))
                {
                    PWSTR psz_folder_path = nullptr;
                    hr = item->GetDisplayName(SIGDN_FILESYSPATH, &psz_folder_path);
                    if (SUCCEEDED(hr))
                    {
                        acul::string folder_path =
                            acul::utf16_to_utf8(reinterpret_cast<const acul::u16string::value_type *>(psz_folder_path));
                        CoTaskMemFree(psz_folder_path);
                        item->Release();
                        file_open_dlg->Release();
                        return folder_path;
                    }
                    item->Release();
                }
            }

            file_open_dlg->Release();
            return {};
        }

        void create_default_path_or_folder(const char *path, IFileDialog *file_dlg)
        {
            if (!path || strlen(path) <= 0) return;
            auto w_default_path = acul::utf8_to_utf16(path);

            acul::io::path fs_path(path);
            auto parent_path = fs_path.parent_path().str();
            if (!parent_path.empty())
            {
                auto w_parent_path = acul::utf8_to_utf16(parent_path);
                IShellItem *default_folder = nullptr;
                HRESULT hr = SHCreateItemFromParsingName(reinterpret_cast<const wchar_t *>(w_parent_path.c_str()), NULL,
                                                         IID_PPV_ARGS(&default_folder));
                if (SUCCEEDED(hr))
                {
                    file_dlg->SetFolder(default_folder);
                    default_folder->Release();
                }
            }

            auto filename = fs_path.filename();
            if (!filename.empty())
            {
                auto w_filename = acul::utf8_to_utf16(filename);
                file_dlg->SetFileName(reinterpret_cast<const wchar_t *>(w_filename.c_str()));
            }
        }

        struct ComFilter
        {
            const acul::vector<FilePattern> *pattern;
            acul::vector<COMDLG_FILTERSPEC> com;
            acul::vector<acul::u16string> descriptions;
            acul::vector<acul::u16string> specs;

            ComFilter(const acul::vector<FilePattern> *pattern) : pattern(pattern)
            {
                com.resize(pattern->size());
                descriptions.resize(pattern->size());
                specs.resize(pattern->size());
            }
        };

        void prepare_com_filter_pattern(ComFilter &filter, IFileDialog *file_dlg)
        {
            assert(filter.pattern);
            auto &pattern = *filter.pattern;
            for (size_t i = 0; i < filter.pattern->size(); ++i)
            {
                filter.descriptions[i] = acul::utf8_to_utf16(pattern[i].description);
                filter.com[i].pszName = reinterpret_cast<const wchar_t *>(filter.descriptions[i].c_str());
                acul::u16string extensions;
                for (size_t j = 0; j < pattern[i].extensions.size(); ++j)
                {
                    if (j > 0) extensions += u";";
                    extensions += acul::utf8_to_utf16(pattern[i].extensions[j]);
                }
                filter.specs[i] = extensions;
                filter.com[i].pszSpec = reinterpret_cast<const wchar_t *>(filter.specs[i].c_str());
            }
            file_dlg->SetFileTypes(static_cast<UINT>(filter.com.size()), filter.com.data());
            file_dlg->SetFileTypeIndex(1);

            if (pattern.empty() || pattern[0].extensions.empty()) return;
            acul::string default_extension = acul::io::get_extension(pattern[0].extensions[0]);
            if (!default_extension.empty() && default_extension[0] == '.')
                default_extension = default_extension.substr(1);
            if (!default_extension.empty())
            {
                auto w_default_extension = acul::utf8_to_utf16(default_extension);
                file_dlg->SetDefaultExtension(reinterpret_cast<const wchar_t *>(w_default_extension.c_str()));
            }
        }

        acul::string save_file_dialog(const char *title, const acul::vector<FilePattern> &pattern,
                                      const char *default_path)
        {
            IFileSaveDialog *file_save_dlg = nullptr;
            HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog,
                                          reinterpret_cast<void **>(&file_save_dlg));
            if (FAILED(hr)) return "";

            if (title)
            {
                auto w_title = acul::utf8_to_utf16(title);
                file_save_dlg->SetTitle(reinterpret_cast<const wchar_t *>(w_title.c_str()));
            }

            create_default_path_or_folder(default_path, file_save_dlg);
            ComFilter com_filter{&pattern};
            prepare_com_filter_pattern(com_filter, file_save_dlg);

            hr = file_save_dlg->Show(nullptr);
            if (SUCCEEDED(hr))
            {
                IShellItem *item = nullptr;
                hr = file_save_dlg->GetResult(&item);
                if (SUCCEEDED(hr))
                {
                    PWSTR psz_file_path = nullptr;
                    hr = item->GetDisplayName(SIGDN_FILESYSPATH, &psz_file_path);

                    if (SUCCEEDED(hr))
                    {
                        acul::string result =
                            acul::utf16_to_utf8(reinterpret_cast<const acul::u16string::value_type *>(psz_file_path));
                        CoTaskMemFree(psz_file_path);
                        item->Release();
                        file_save_dlg->Release();
                        return result;
                    }
                    item->Release();
                }
            }
            file_save_dlg->Release();
            return "";
        }

    } // namespace popup
} // namespace awin