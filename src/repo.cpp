#include "repo.h"
#include "config.h"
#include "displays.h"
#include "log.h"
#include "paths.h"
#include "util.h"

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
    WF_LOG(LogLevel::LINFO, std::format("retrieving wallpaper path for repo ({})", repo_key));
    return std::format("{}\\{}", config->wallpaperDir, repo_key);
}

std::mutex populate_repo_mtx;

bool IsSupportedFormat(IIFormat format)
{
    switch (format) {
    case II_FORMAT_PNG:
        return true;
    default:
        return false;
    }
}

std::vector<std::string> GetVerifiedImages(std::vector<std::string> files, uint16_t width, uint16_t height)
{
    std::vector<std::string> result;
    for (int i = 0; i < files.size(); i++) {
        WF_LOG(LogLevel::LINFO, std::format("verifying image {}", files[i]));
        ImageInfo info = getImageInfo<IIFilePathReader>(files[i]);
        IIFormat format = info.getFormat();

        if (!IsSupportedFormat(format)) {
            WF_LOG(LogLevel::LWARNING, "image (" + files[i] + ") unsupported format");
            continue;
        }

        if (info.getWidth() != width || info.getHeight() != height) {
            WF_LOG(LogLevel::LWARNING, "image (" + files[i] + ") invalid size");
            continue;
        }

        WF_LOG(LogLevel::LINFO, std::format("verified image {}", files[i]));

        result.push_back(files[i]);
    }
    return result;
}

std::vector<std::string> GetValidImageFiles(std::string repo_path, uint16_t width, uint16_t height)
{
    WF_LOG(LogLevel::LINFO, std::format("retrieving valid image files in directory ()", repo_path));
    std::vector<std::string> allowed_extensions = { "png" };
    std::vector<std::string> unverified_image_files = GetFilesWithExtensions(repo_path, allowed_extensions);
    return GetVerifiedImages(unverified_image_files, width, height);
}

std::string GetRepoKey(uint16_t width, uint16_t height)
{
    return std::format("{}x{}", width, height);
}

void CreateRepoDirIfNotFound(std::string path)
{
    WF_LOG(LogLevel::LINFO, std::format("creating {} if not found", path));
    if (std::filesystem::exists(path)) {
        if (!std::filesystem::is_directory(path)) {
            throw std::runtime_error("expected wallpaper path (" + path + ") to be a directory, it is not.");
        }
        return;
    }

    if (!std::filesystem::create_directory(path)) {
        throw std::runtime_error("could not create wallpaper directory (" + path + ")");
    }
}

void ShuffleImageFiles(std::vector<std::string>& image_files)
{
    WF_LOG(LogLevel::LINFO, "shuffling image files");
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(image_files.begin(), image_files.end(), rng);
}

void PopulateRepo(uint16_t width, uint16_t height)
{
    std::lock_guard<std::mutex> lock(populate_repo_mtx);

    std::string key = GetRepoKey(width, height);
    WF_LOG(LogLevel::LINFO, std::format("populating repo ()", key));

    repo_indexes[key] = -1;

    std::string repo_path = GetRepoPath(key);
    CreateRepoDirIfNotFound(repo_path);

    std::vector<std::string> image_files = GetValidImageFiles(repo_path, width, height);

    if (config->shuffle) {
        ShuffleImageFiles(image_files);
    }

    for (int i = 0; i < image_files.size(); i++) {
        WF_LOG(LogLevel::LINFO, "image loaded (" + image_files[i] + ")");
    }

    repo_files[key] = image_files;
}

void PopulateAllRepos()
{
    WF_START_TIMER("PopulateAllRepos()");
    std::map<std::string, Display> unique_display_sizes;

    for (Display display : displays) {
        unique_display_sizes[display.repoKey] = display;
    }

    for (const auto& pair : unique_display_sizes) {
        PopulateRepo(pair.second.width, pair.second.height);
    }
    WF_END_TIMER("PopulateAllRepos()");
}

bool FilesHaveChanged(uint16_t width, uint16_t height)
{
    std::string key = GetRepoKey(width, height);
    std::string repo_path = GetRepoPath(key);

    return !StringVectorsMatch(
        repo_files[key],
        GetValidImageFiles(repo_path, width, height));
}

std::string GetNextImage(uint16_t width, uint16_t height)
{
    std::string key = GetRepoKey(width, height);
    WF_LOG(LogLevel::LINFO, std::format("retrieving next image for repo ()", key));

    if (FilesHaveChanged(width, height)) {
        WF_LOG(LogLevel::LINFO, std::format("files for repo {} have changed, repopulating", key));
        PopulateRepo(width, height);
    }

    if (repo_files[key].size() == 0) {
        WF_LOG(LogLevel::LINFO, std::format("no images found for repo ()", key));
        return "";
    }

    repo_indexes[key] = repo_indexes[key] + 1;
    int index = repo_indexes[key] % repo_files[key].size();

    WF_LOG(LogLevel::LINFO, std::format("found image ()", repo_files[key][index]));
    return repo_files[key][index];
}

}