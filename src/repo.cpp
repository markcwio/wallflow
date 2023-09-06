#include "repo.h"
#include "config.h"
#include "displays.h"
#include "log.h"
#include "paths.h"

#include <imageinfo.hpp>

#include <algorithm>
#include <filesystem>
#include <format>
#include <random>

namespace wallflow {

std::map<std::string, int> repo_indexes;
std::map<std::string, std::vector<std::string>> repo_files;

std::string GetRepoPath(std::string repo_key)
{
    return std::format("{}\\{}", config->wallpaperDir, repo_key);
}

std::mutex populate_repo_mtx;

void PopulateRepo(uint16_t width, uint16_t height)
{
    std::lock_guard<std::mutex> lock(populate_repo_mtx);

    std::string key = std::format("{}x{}", width, height);
    WF_LOG(LogLevel::LINFO, "populating repo " + key);

    repo_indexes[key] = 0;

    std::string repo_path = GetRepoPath(key);

    do {
        if (std::filesystem::exists(repo_path)) {
            if (!std::filesystem::is_directory(repo_path)) {
                throw std::runtime_error("expected wallpaper path (" + repo_path + ") to be a folder, it is not.");
            }
            break;
        }

        if (std::filesystem::create_directory(repo_path)) {
            throw std::runtime_error("could not create wallpaper folder (" + repo_path + ")");
        }
    } while (false);

    std::vector<std::string> allowed_extensions = { "jpg", "jpeg", "png", "bmp" };
    std::vector<std::string> image_files = GetFilesWithExtensions(repo_path, allowed_extensions);

    for (int i = 0; i < image_files.size(); i++) {
        ImageInfo info = getImageInfo<IIFilePathReader>(image_files[i]);
        if (info.getWidth() != width) {
            WF_LOG(LogLevel::LWARNING, "image (" + image_files[i] + ") invalid size");
            image_files.erase(image_files.begin() + i);
        }
    }

    if (config->shuffle) {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(image_files.begin(), image_files.end(), rng);
    }

    for (int i = 0; i < image_files.size(); i++) {
        WF_LOG(LogLevel::LINFO, "image loaded (" + image_files[i] + ")");
    }

    repo_files[key] = image_files;
}

void PopulateAllRepos()
{
    WF_START_TIMER("PopulateAllRepos()");
    std::vector<std::string> keys;
    std::map<std::string, Display> unique_display_sizes;

    for (Display display : displays) {
        unique_display_sizes[display.repoKey] = display;
    }

    for (const auto& pair : unique_display_sizes) {
        PopulateRepo(pair.second.width, pair.second.height);
    }
    WF_END_TIMER("PopulateAllRepos()");
}

std::string GetNextImage(uint16_t width, uint16_t height)
{
    std::string key = std::format("{}x{}", width, height);
    repo_indexes[key] = repo_indexes[key] + 1;
    int index = repo_indexes[key] % repo_files[key].size();

    return repo_files[key][index];
}

}