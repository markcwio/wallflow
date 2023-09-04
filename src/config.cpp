
#include "config.h"
#include "log.h"
#include "paths.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

namespace wallflow {

Config* config;
std::time_t last_modified_at;

std::string Config::ToString()
{
    return std::format(
        "Config(wallpaperDir={},changeInterval={},shuffle={},imageFormat={},jpegQuality={})",
        wallpaperDir,
        changeInterval,
        shuffle,
        imageFormat,
        jpegQuality);
}

std::string getConfigPath()
{
    return GetAppDataPath("config.json");
}

void LoadConfig()
{
    WF_LOG(LogLevel::LINFO, "loading config file");
    WF_START_TIMER("LoadConfig");

    std::string config_path = getConfigPath();

    if (!std::filesystem::exists(config_path)) {
        CreateDefaultConfig();
    }

    std::string in_path = getConfigPath();
    std::ifstream in_file(in_path);

    if (!in_file.is_open()) {
        throw std::runtime_error("could not open config file to read");
    }

    nlohmann::json json_config = nlohmann::json::parse(in_file);

    config = new Config;
    config->wallpaperDir = json_config["wallpaperDir"];
    config->changeInterval = json_config["changeInterval"];
    config->shuffle = json_config["shuffle"];
    config->imageFormat = json_config["imageFormat"];
    config->jpegQuality = json_config["jpegQuality"];

    WF_END_TIMER("LoadConfig");
    WF_LOG_OBJ_PTR(config);
}

bool LoadConfigIfModified(

);

void CreateDefaultConfig()
{
    nlohmann::json config_json;

    config_json["wallpaperDir"] = GetUserPath("Pictures\\Wallpapers");
    config_json["changeInterval"] = 300;
    config_json["shuffle"] = true;
    config_json["imageFormat"] = "jpg";
    config_json["jpegQuality"] = 95;

    std::string out_path = getConfigPath();
    std::ofstream out_file(out_path);

    if (!out_file.is_open()) {
        throw std::runtime_error("could not open config file to write defaults");
    }

    out_file << std::setw(4) << config_json << std::endl;

    out_file.close();
}

void SaveConfig()
{
}

}