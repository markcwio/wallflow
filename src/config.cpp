
#include "config.h"
#include "log.h"
#include "paths.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>

#include <nlohmann/json.hpp>

namespace wallflow {

Config* config;
std::time_t last_modified_at;

std::string Config::ToString()
{
    return std::format(
        "Config(wallpaperDir={},cycleSpeed={},shuffle={})",
        wallpaperDir,
        cycleSpeed,
        shuffle);
}

std::string GetConfigPath()
{
    return GetAppDataPath("config.json");
}

std::time_t getConfigModifiedTime()
{
    struct stat file_stat;

    if (stat(GetConfigPath().c_str(), &file_stat) == 0) {
        return file_stat.st_mtime;
    }

    throw std::runtime_error("could not get mofication date of config file");
}

std::mutex load_config_mtx;

void LoadConfig()
{
    std::lock_guard<std::mutex> lock(load_config_mtx);

    WF_LOG(LogLevel::LINFO, "loading config file");
    WF_START_TIMER("LoadConfig()");

    std::string config_path = GetConfigPath();

    if (!std::filesystem::exists(config_path)) {
        CreateDefaultConfig();
    }

    std::string in_path = GetConfigPath();
    std::ifstream in_file(in_path);

    if (!in_file.is_open()) {
        throw std::runtime_error("could not open config file to read");
    }

    nlohmann::json json_config = nlohmann::json::parse(in_file);

    if (config == nullptr) {
        config = new Config;
    }
    config->wallpaperDir = json_config["wallpaperDir"];
    config->cycleSpeed = json_config["cycleSpeed"];
    config->shuffle = json_config["shuffle"];

    last_modified_at = getConfigModifiedTime();

    WF_END_TIMER("LoadConfig()");
    WF_LOG_OBJ_PTR(config);
}

bool LoadConfigIfModified()
{
    std::lock_guard<std::mutex> lock(load_config_mtx);

    WF_LOG(LogLevel::LINFO, "loading config file if modified");
    WF_START_TIMER("LoadConfigIfModified()");

    std::time_t modified_at = getConfigModifiedTime();

    if (modified_at == last_modified_at) {
        WF_LOG(LogLevel::LINFO, "config file has not been modified");
        WF_END_TIMER("LoadConfigIfModified()");
        return false;
    }

    WF_LOG(LogLevel::LINFO, "config file has been modified");

    LoadConfig();

    WF_END_TIMER("LoadConfigIfModified()");

    return true;
};

void CreateDefaultConfig()
{
    WF_LOG(LogLevel::LINFO, "creating default config file");
    WF_START_TIMER("CreateDefaultConfig()");

    nlohmann::json config_json;

    std::string wallpaper_path = SelectDirectoryDialog();

    if (wallpaper_path == "") {
        throw std::runtime_error("wallpaper directory must be selected");
    }

    WF_LOG(LogLevel::LINFO, std::format("wallpaper directory selected ({})", wallpaper_path));

    config_json["wallpaperDir"] = wallpaper_path;
    config_json["cycleSpeed"] = 300;
    config_json["shuffle"] = true;
    config_json["imageFormat"] = "jpg";
    config_json["jpegQuality"] = 95;

    std::string out_path = GetConfigPath();
    std::ofstream out_file(out_path);

    if (!out_file.is_open()) {
        WF_END_TIMER("CreateDefaultConfig()");
        throw std::runtime_error("could not open config file to write defaults");
    }

    out_file << std::setw(4) << config_json << std::endl;
    out_file.close();

    WF_END_TIMER("CreateDefaultConfig()");
}

std::mutex save_config_mtx;

void SaveConfig()
{
    std::lock_guard<std::mutex> lock(save_config_mtx);

    WF_LOG(LogLevel::LINFO, "saving config file");
    WF_START_TIMER("SaveConfig()");

    nlohmann::json config_json;

    config_json["wallpaperDir"] = config->wallpaperDir;
    config_json["cycleSpeed"] = config->cycleSpeed;
    config_json["shuffle"] = config->shuffle;

    std::string out_path = GetConfigPath();
    std::ofstream out_file(out_path);

    if (!out_file.is_open()) {
        WF_END_TIMER("SaveConfig()");
        throw std::runtime_error("could not open config file to write changes");
    }

    out_file << std::setw(4) << config_json << std::endl;
    out_file.close();

    WF_END_TIMER("SaveConfig()");
}

std::mutex display_alias_mtx;

std::string GetDisplayAliasPath()
{
    WF_LOG(LogLevel::LINFO, "retrieving display alias path");
    return GetAppDataPath("display_aliases.json");
}

void CreateDisplayAliasFileIfNotFound()
{
    WF_LOG(LogLevel::LINFO, "creating display alias file if no file is found");

    std::string alias_path = GetDisplayAliasPath();

    if (std::filesystem::exists(alias_path)) {
        return;
    }

    std::ofstream out_file(alias_path);

    if (!out_file.is_open()) {
        throw std::runtime_error("could not open display aliases file to write");
    }

    nlohmann::json alias_json;
    out_file << std::setw(4) << alias_json << std::endl;
}

std::string GetDisplayAlias(std::string id)
{
    WF_LOG(LogLevel::LINFO, std::format("retrieving display alias ({})", id));

    std::lock_guard<std::mutex> lock(display_alias_mtx);
    CreateDisplayAliasFileIfNotFound();

    std::string in_path = GetDisplayAliasPath();
    std::ifstream in_file(in_path);

    if (!in_file.is_open()) {
        throw std::runtime_error("could not open display alias file to read");
    }

    nlohmann::json alias_json = nlohmann::json::parse(in_file);

    if (alias_json[id].is_string()) {
        return alias_json[id];
    }

    return "";
}

void SaveDisplayAlias(std::string id, std::string alias)
{
    WF_LOG(LogLevel::LINFO, std::format("saving display alias (id={},alias={})", id, alias));

    std::lock_guard<std::mutex> lock(display_alias_mtx);
    CreateDisplayAliasFileIfNotFound();

    std::string alias_path = GetDisplayAliasPath();
    std::ifstream in_file(alias_path);

    if (!in_file.is_open()) {
        throw std::runtime_error("could not open display alias file to read");
    }

    nlohmann::json alias_json = nlohmann::json::parse(in_file);

    alias_json[id] = alias;

    std::ofstream out_file(alias_path);

    if (!out_file.is_open()) {
        throw std::runtime_error("could not open display aliases file to write");
    }

    out_file << std::setw(4) << alias_json << std::endl;
}

std::string GetOrCreateAlias(std::string id, std::string alias)
{
    WF_LOG(LogLevel::LINFO, std::format("retrieving or creating alias for display (id={},alias={})", id, alias));

    std::string result = GetDisplayAlias(id);
    if (result != "") {
        WF_LOG(LogLevel::LINFO, std::format("could not find alias for ({}) using default ({})", id, alias));
        return result;
    }

    SaveDisplayAlias(id, alias);
    return alias;
}

void ChangeWallpaperDir()
{
    WF_LOG(LogLevel::LINFO, "chaning wallpaper directory");

    std::string wallpaper_path = SelectDirectoryDialog();

    if (wallpaper_path == "") {
        WF_LOG(LogLevel::LINFO, "no wallpaper directory selected");
        return;
    }

    config->wallpaperDir = wallpaper_path;
    SaveConfig();

    WF_LOG(LogLevel::LINFO, std::format("wallpaper directory changed to ({})", config->wallpaperDir));
}

void ToggleShuffle()
{
    config->shuffle = !config->shuffle;
    SaveConfig();
}

void SetCycleSpeed(int value)
{
    config->cycleSpeed = value;
    SaveConfig();
}

}