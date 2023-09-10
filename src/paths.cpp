#include "paths.h"
#include "convert.h"
#include "log.h"

#include <filesystem>
#include <format>
#include <iostream>
#include <sstream>

#include <shlobj.h>
#include <windows.h>

namespace wallflow {

void CreateAppDataDir()
{
    WF_LOG(LogLevel::LINFO, "creating AppData directory");
    WF_START_TIMER("CreateAppDataDir()");

    try {
        std::string dir_path = GetAppDataDir();

        if (std::filesystem::exists(dir_path)) {
            if (!std::filesystem::is_directory(dir_path)) {
                throw std::runtime_error("expected AppData path to be directory, it is not.");
            }
            WF_END_TIMER("CreateAppDataDir()");
            return;
        }

        if (!std::filesystem::create_directory(dir_path)) {
            throw std::exception("could not create AppData directory");
        }

    } catch (const std::exception& ex) {
        WF_END_TIMER("CreateAppDataDir()");
        throw;
    }

    WF_END_TIMER("CreateAppDataDir()");
}

std::string GetAppDataDir()
{
    PWSTR pszPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &pszPath))) {
        std::wstring appDataPath(pszPath);
        CoTaskMemFree(pszPath);
        std::wstringstream ws;
        ws << appDataPath << "\\" << APP_DATA_DIR;
        std::wstring ws_str = ws.str();
        return WStringToString(ws_str);
    }

    throw std::runtime_error("could not create AppData directory");
}

std::string GetAppDataPath(std::string path)
{
    return std::format("{}\\{}", GetAppDataDir(), path);
}

std::string GetUserDir()
{
    wchar_t user_wpath[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, user_wpath))) {
        throw std::exception("Could not get users profile directory");
    }

    std::wstring ws(user_wpath);
    return WStringToString(ws);
}

std::string GetUserPath(std::string path)
{
    return std::format("{}\\{}", GetUserDir(), path);
}

std::string SelectDirectoryDialog()
{
    CoInitialize(NULL);

    std::wstring selected_dir;

    IFileDialog* p_file_dialog = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p_file_dialog));

    if (SUCCEEDED(hr)) {
        DWORD dwOptions;
        p_file_dialog->GetOptions(&dwOptions);
        p_file_dialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

        hr = p_file_dialog->Show(NULL);

        if (SUCCEEDED(hr)) {
            IShellItem* p_shell_item;
            hr = p_file_dialog->GetResult(&p_shell_item);

            if (SUCCEEDED(hr)) {
                PWSTR dir_path;
                hr = p_shell_item->GetDisplayName(SIGDN_FILESYSPATH, &dir_path);

                if (SUCCEEDED(hr)) {
                    selected_dir = dir_path;
                    CoTaskMemFree(dir_path);
                }
                p_shell_item->Release();
            }
        }

        p_file_dialog->Release();
    }

    CoUninitialize();

    return WStringToString(selected_dir);
}

std::vector<std::string> GetFilesWithExtensions(const std::string& dir_path, const std::vector<std::string>& extensions)
{
    std::vector<std::string> files;

    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::string file_path = entry.path().string();
        for (const std::string& ext : extensions) {
            if (file_path.ends_with("." + ext)) {
                files.push_back(file_path);
                break;
            }
        }
    }

    return files;
}

}