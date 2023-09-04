#pragma once

#include <string>

namespace wallflow {

struct Config {
    std::string wallpaperDir;
    unsigned int changeInterval;
    bool shuffle;
    std::string imageFormat;
    unsigned int jpegQuality;
    std::string ToString();
};

extern Config* config;

void LoadConfig();
bool LoadConfigIfModified();
void CreateDefaultConfig();
void SaveConfig();

}