#include <astl/string.hpp>
#include <filesystem>
#include <window/popup.hpp>
#include <windows.h>

// Include windows first
#include <commdlg.h>
#include <shlobj.h>
#include <wchar.h>

#define BTN_OK     0x1
#define BTN_CANCEL 0x2
#define BTN_YES    0x4
#define BTN_NO     0x8

namespace window
{
    namespace popup
    {
        inline int styleToIcon(Style style)
        {
            switch (style)
            {
                case Style::Info:
                    return MB_ICONINFORMATION;
                case Style::Warning:
                    return MB_ICONWARNING;
                case Style::Error:
                    return MB_ICONERROR;
                case Style::Question:
                    return MB_ICONQUESTION;
            }
            return MB_ICONINFORMATION;
        }

        int buttonsToFlags(std::initializer_list<Buttons> buttons)
        {
            int mask = 0x0;
            for (auto btn : buttons)
            {
                switch (btn)
                {
                    case Buttons::OK:
                        mask |= BTN_OK;
                        break;
                    case Buttons::Yes:
                        mask |= BTN_YES;
                        break;
                    case Buttons::No:
                        mask |= BTN_NO;
                        break;
                    case Buttons::Cancel:
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

        Buttons msgBox(const char *message, const char *title, Style style, std::initializer_list<Buttons> buttons,
                       window::Window *parent)
        {
            UINT iconFlag = styleToIcon(style);
            UINT buttonsFlag = buttonsToFlags(buttons);
            UINT flags = buttonsFlag | iconFlag;
            HWND hwnd = parent ? platform::native_access::getHWND(*parent) : nullptr;
            if (!hwnd) flags |= MB_TOPMOST;
            auto wMessage = astl::utf8_to_utf16(message);
            LPCWSTR lpText = reinterpret_cast<LPCWSTR>(wMessage.c_str());
            auto wTitle = astl::utf8_to_utf16(title);
            LPCWSTR lpCaption = reinterpret_cast<LPCWSTR>(wTitle.c_str());
            switch (MessageBoxW(hwnd, lpText, lpCaption, flags))
            {
                case IDOK:
                    return Buttons::OK;
                case IDYES:
                    return Buttons::Yes;
                case IDNO:
                    return Buttons::No;
                case IDCANCEL:
                    return Buttons::Cancel;
                default:
                    return Buttons::Error;
            };
        }

        std::string openFileDialog(const char *title, const astl::vector<FilePattern> &pattern, const char *defaultPath,
                                   bool multiply)
        {
            std::u16string wTitle = astl::utf8_to_utf16(title ? std::string(title) : "");
            std::u16string wDefaultPath = defaultPath ? astl::utf8_to_utf16(std::string(defaultPath)) : u"";

            std::u16string wFilterPatterns;
            for (const auto &pat : pattern)
            {
                wFilterPatterns += astl::utf8_to_utf16(pat.description) + u'\0';
                for (size_t i = 0; i < pat.extensions.size(); ++i)
                {
                    wFilterPatterns += astl::utf8_to_utf16(pat.extensions[i]);
                    if (i < pat.extensions.size() - 1) wFilterPatterns += u';';
                }
                wFilterPatterns += u'\0';
            }
            wFilterPatterns += u'\0';

            wchar_t lFileName[MAX_PATH] = L"";
            OPENFILENAMEW ofn = {0};
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = GetForegroundWindow();
            ofn.lpstrFilter = reinterpret_cast<LPCWSTR>(wFilterPatterns.c_str()); // Преобразование для совместимости
            ofn.lpstrFile = lFileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = reinterpret_cast<LPCWSTR>(wTitle.c_str());
            ofn.lpstrInitialDir = wDefaultPath.empty() ? NULL : reinterpret_cast<LPCWSTR>(wDefaultPath.c_str());
            ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (multiply) ofn.Flags |= OFN_ALLOWMULTISELECT;
            if (GetOpenFileNameW(&ofn) == 0) return "";
            return astl::utf16_to_utf8(reinterpret_cast<const std::u16string::value_type *>(lFileName));
        }

        std::string openFolderDialog(const char *title, const char *defaultPath)
        {
            IFileOpenDialog *pFileOpen = nullptr;

            HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                                          reinterpret_cast<void **>(&pFileOpen));
            if (FAILED(hr))
            {
                CoUninitialize();
                return "";
            }

            DWORD dwOptions;
            hr = pFileOpen->GetOptions(&dwOptions);
            if (SUCCEEDED(hr)) pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);

            if (title)
            {
                std::u16string wTitle = astl::utf8_to_utf16(title);
                pFileOpen->SetTitle(reinterpret_cast<LPCWSTR>(wTitle.c_str()));
            }

            if (defaultPath)
            {
                std::u16string wDefaultPath = astl::utf8_to_utf16(defaultPath);
                IShellItem *pItem = nullptr;
                hr = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(wDefaultPath.c_str()), NULL,
                                                 IID_PPV_ARGS(&pItem));
                if (SUCCEEDED(hr))
                {
                    pFileOpen->SetFolder(pItem);
                    pItem->Release();
                }
            }

            hr = pFileOpen->Show(NULL);
            if (SUCCEEDED(hr))
            {
                IShellItem *pItem = nullptr;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFolderPath = nullptr;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
                    if (SUCCEEDED(hr))
                    {
                        std::string folderPath =
                            astl::utf16_to_utf8(reinterpret_cast<const std::u16string::value_type *>(pszFolderPath));
                        CoTaskMemFree(pszFolderPath);
                        pItem->Release();
                        pFileOpen->Release();
                        return folderPath;
                    }
                    pItem->Release();
                }
            }

            pFileOpen->Release();
            return "";
        }

        void createDefaultPathOrFolder(const char *path, IFileDialog *pFile)
        {
            if (!path || strlen(path) <= 0) return;
            auto wDefaultPath = astl::utf8_to_utf16(path);
            DWORD attributes = GetFileAttributesW(reinterpret_cast<const wchar_t *>(wDefaultPath.c_str()));

            std::filesystem::path fsPath(path);
            auto parentPath = fsPath.parent_path().string();
            if (!parentPath.empty())
            {
                auto wParentPath = astl::utf8_to_utf16(parentPath);
                IShellItem *pDefaultFolder = nullptr;
                HRESULT hr = SHCreateItemFromParsingName(reinterpret_cast<const wchar_t *>(wParentPath.c_str()), NULL,
                                                         IID_PPV_ARGS(&pDefaultFolder));
                if (SUCCEEDED(hr))
                {
                    pFile->SetFolder(pDefaultFolder);
                    pDefaultFolder->Release();
                }
            }

            auto fileName = fsPath.filename().string();
            if (!fileName.empty())
            {
                auto wFileName = astl::utf8_to_utf16(fileName);
                pFile->SetFileName(reinterpret_cast<const wchar_t *>(wFileName.c_str()));
            }
        }

        struct ComFilter
        {
            const astl::vector<FilePattern> *pattern;
            astl::vector<COMDLG_FILTERSPEC> com;
            astl::vector<std::u16string> descriptions;
            astl::vector<std::u16string> specs;

            ComFilter(const astl::vector<FilePattern> *pattern) : pattern(pattern)
            {
                com.resize(pattern->size());
                descriptions.resize(pattern->size());
                specs.resize(pattern->size());
            }
        };

        void prepareComFilterPattern(ComFilter &filter, IFileDialog *pFile)
        {
            assert(filter.pattern);
            auto &pattern = *filter.pattern;
            for (size_t i = 0; i < filter.pattern->size(); ++i)
            {
                filter.descriptions[i] = astl::utf8_to_utf16(pattern[i].description);
                filter.com[i].pszName = reinterpret_cast<const wchar_t *>(filter.descriptions[i].c_str());
                std::u16string extensions;
                for (size_t j = 0; j < pattern[i].extensions.size(); ++j)
                {
                    if (j > 0) extensions += u";";
                    extensions += astl::utf8_to_utf16(pattern[i].extensions[j]);
                }
                filter.specs[i] = extensions;
                filter.com[i].pszSpec = reinterpret_cast<const wchar_t *>(filter.specs[i].c_str());
            }
            pFile->SetFileTypes(static_cast<UINT>(filter.com.size()), filter.com.data());
            pFile->SetFileTypeIndex(1);

            if (pattern.empty() || pattern[0].extensions.empty()) return;
            std::filesystem::path extPath(pattern[0].extensions[0]);
            std::string defExt = extPath.extension().string();
            if (!defExt.empty() && defExt[0] == '.') defExt = defExt.substr(1);
            if (!defExt.empty())
            {
                auto wDefExt = astl::utf8_to_utf16(defExt);
                pFile->SetDefaultExtension(reinterpret_cast<const wchar_t *>(wDefExt.c_str()));
            }
        }

        std::string saveFileDialog(const char *title, const astl::vector<FilePattern> &pattern, const char *defaultPath)
        {
            IFileSaveDialog *pFileSave = nullptr;
            HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog,
                                          reinterpret_cast<void **>(&pFileSave));
            if (FAILED(hr)) return "";

            if (title)
            {
                auto wTitle = astl::utf8_to_utf16(title);
                pFileSave->SetTitle(reinterpret_cast<const wchar_t *>(wTitle.c_str()));
            }

            createDefaultPathOrFolder(defaultPath, pFileSave);
            ComFilter comFilter{&pattern};
            prepareComFilterPattern(comFilter, pFileSave);

            hr = pFileSave->Show(nullptr);
            if (SUCCEEDED(hr))
            {
                IShellItem *pItem = nullptr;
                hr = pFileSave->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath = nullptr;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    if (SUCCEEDED(hr))
                    {
                        std::string result =
                            astl::utf16_to_utf8(reinterpret_cast<const std::u16string::value_type *>(pszFilePath));
                        CoTaskMemFree(pszFilePath);
                        pItem->Release();
                        pFileSave->Release();
                        return result;
                    }
                    pItem->Release();
                }
            }
            pFileSave->Release();
            return "";
        }

    } // namespace popup
} // namespace window