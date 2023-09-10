#pragma once

#include <string>

namespace wallflow {

struct Config {
    std::string wallpaperDir;
    unsigned int cycleSpeed;
    bool shuffle;
    std::string ToString();
};

extern Config* config;

std::string GetConfigPath();
std::string GetDisplayAliasPath();
void LoadConfig();
bool LoadConfigIfModified();
void CreateDefaultConfig();
void SaveConfig();
void CreateDisplayAliasFileIfNotFound();
std::string GetDisplayAlias(std::string id);
void SaveDisplayAlias(std::string id, std::string alias);
std::string GetOrCreateAlias(std::string id, std::string alias);
void ChangeWallpaperDir();
void ToggleShuffle();
void SetCycleSpeed(int value);
}