
#include "paths.h"
#include "convert.h"
#include "log.h"

#include <filesystem>
#include <format>
#include <iostream>
#include <sstream>

#include <Shlobj.h>
#include <Windows.h>

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

}