#pragma once

#include <string>
#include <vector>

#ifndef APP_DATA_DIR
#define APP_DATA_DIR "WallFlow"
#endif

namespace wallflow {

void CreateAppDataDir();
std::string GetAppDataDir();
std::string GetAppDataPath(std::string path);
std::string GetUserDir();
std::string GetUserPath(std::string path);
std::string SelectDirectoryDialog();
std::vector<std::string> GetFilesWithExtensions(const std::string& dir_path, const std::vector<std::string>& extensions);
}