#include <core/std/string.hpp>
#include <window/platform_win32.hpp>
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
            HWND hwnd = parent ? parent->accessBridge().hwnd() : nullptr;
            if (!hwnd) flags |= MB_TOPMOST;
            auto wMessage = convertUTF8toUTF16(message);
            LPCWSTR lpText = reinterpret_cast<LPCWSTR>(wMessage.c_str());
            auto wTitle = convertUTF8toUTF16(title);
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

        std::string openFileDialog(const char *title, const DArray<FilePattern> &pattern, const char *defaultPath,
                                   bool multiply)
        {
            std::u16string wTitle = convertUTF8toUTF16(title ? std::string(title) : "");
            std::u16string wDefaultPath = defaultPath ? convertUTF8toUTF16(std::string(defaultPath)) : u"";

            std::u16string wFilterPatterns;
            for (const auto &pat : pattern)
            {
                wFilterPatterns += convertUTF8toUTF16(pat.description) + u'\0';
                for (size_t i = 0; i < pat.extensions.size(); ++i)
                {
                    wFilterPatterns += convertUTF8toUTF16(pat.extensions[i]);
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
            return convertUTF16toUTF8(reinterpret_cast<const std::u16string::value_type *>(lFileName));
        }

        std::string openFolderDialog(const char *title, const char *defaultPath)
        {
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (FAILED(hr)) { return ""; }

            IFileOpenDialog *pFileOpen = nullptr;

            hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
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
                std::u16string wTitle = convertUTF8toUTF16(title);
                pFileOpen->SetTitle(reinterpret_cast<LPCWSTR>(wTitle.c_str()));
            }

            if (defaultPath)
            {
                std::u16string wDefaultPath = convertUTF8toUTF16(defaultPath);
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
                            convertUTF16toUTF8(reinterpret_cast<const std::u16string::value_type *>(pszFolderPath));
                        CoTaskMemFree(pszFolderPath);
                        pItem->Release();
                        pFileOpen->Release();
                        CoUninitialize();
                        return folderPath;
                    }
                    pItem->Release();
                }
            }

            pFileOpen->Release();
            CoUninitialize();

            return "";
        }

        std::string saveFileDialog(const char *title, const DArray<FilePattern> &pattern, const char *defaultPath)
        {
            std::u16string wTitle = convertUTF8toUTF16(title ? std::string(title) : "");
            std::u16string wDefaultPath = defaultPath ? convertUTF8toUTF16(std::string(defaultPath)) : u"";

            wchar_t lFileName[MAX_PATH] = L"";

            if (defaultPath && strlen(defaultPath) > 0)
            {
                DWORD attributes = GetFileAttributesA(defaultPath);
                if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY))
                    wDefaultPath = convertUTF8toUTF16(defaultPath);
                else
                {
                    std::u16string wDefaultFileName = convertUTF8toUTF16(defaultPath);
                    wcscpy_s(lFileName, MAX_PATH, reinterpret_cast<const wchar_t *>(wDefaultFileName.c_str()));
                }
            }

            std::u16string wFilterPatterns;
            for (const auto &pat : pattern)
            {
                wFilterPatterns += convertUTF8toUTF16(pat.description) + u'\0';
                for (size_t i = 0; i < pat.extensions.size(); ++i)
                {
                    wFilterPatterns += convertUTF8toUTF16(pat.extensions[i]);
                    if (i < pat.extensions.size() - 1) wFilterPatterns += u';';
                }
                wFilterPatterns += u'\0';
            }
            wFilterPatterns += u'\0';

            OPENFILENAMEW ofn = {0};
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = GetForegroundWindow();
            ofn.lpstrFilter = reinterpret_cast<LPCWSTR>(wFilterPatterns.c_str());
            ofn.lpstrFile = lFileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = reinterpret_cast<LPCWSTR>(wTitle.c_str());
            ofn.lpstrInitialDir = wDefaultPath.empty() ? NULL : reinterpret_cast<LPCWSTR>(wDefaultPath.c_str());
            ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileNameW(&ofn) == 0) return "";
            return convertUTF16toUTF8(reinterpret_cast<const std::u16string::value_type *>(lFileName));
        }
    } // namespace popup
} // namespace window