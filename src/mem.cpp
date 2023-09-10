#include "mem.h"
#include "log.h"
#include "paths.h"

#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <map>
#include <random>

namespace wallflow {

std::map<uint16_t, FileMemoryBuffer> file_memory_buffers;

uint16_t CreateFileMemoryBufferKey()
{
    uint16_t key = 0;
    bool valid_key = false;
    std::random_device rd;
    std::mt19937 gen(rd());
    uint16_t min_value = 10000;
    uint16_t max_value = std::numeric_limits<uint16_t>::max();

    do {

        std::uniform_int_distribution<uint16_t> dis(min_value, max_value);
        uint16_t value = dis(gen);

        std::map<uint16_t, FileMemoryBuffer>::iterator it = file_memory_buffers.find(value);

        if (it == file_memory_buffers.end()) {
            key = value;
        }

    } while (key == 0);

    return key;
}

void DeleteFileMemoryBuffer(uint16_t key)
{
    WF_LOG(LogLevel::LINFO, std::format("deleting file memory buffer ({})", key));

    std::map<uint16_t, FileMemoryBuffer>::iterator it = file_memory_buffers.find(key);
    if (it == file_memory_buffers.end()) {
        WF_LOG(LogLevel::LWARNING, std::format("could not find file memory buffer {}", key));
    }

    FileMemoryBuffer fmb = file_memory_buffers[key];
    UnmapViewOfFile(fmb.pMemory);
    CloseHandle(fmb.hMapFile);
    CloseHandle(fmb.hFile);
    std::filesystem::remove(fmb.filePath);
    file_memory_buffers.erase(it);
}

uint16_t CreateFileMemoryBuffer(size_t size)
{
    uint16_t key = CreateFileMemoryBufferKey();
    WF_LOG(LogLevel::LINFO, std::format("creating file memory buffer ({})", key));

    std::string buffer_path = GetAppDataPath(std::format("{}.dat", key));

    if (std::filesystem::exists(buffer_path)) {
        size_t current_size = std::filesystem::file_size(buffer_path);

        if (current_size != size) {
            WF_LOG(LogLevel::LINFO, std::format("resizing buffer {}.dat {} -> {}", key, current_size, size));
            std::filesystem::resize_file(buffer_path, size);
        } else {
            WF_LOG(LogLevel::LINFO, "buffer file does not need resizing");
        }
    } else {
        WF_LOG(LogLevel::LINFO, std::format("creating buffer {}.dat {}", key, size));

        std::ofstream file(buffer_path);
        if (!file.is_open()) {
            throw std::runtime_error("failed to create output buffer file");
        }
        file.seekp(size - 1);
        file.put(0);
        file.close();
    }

    FileMemoryBuffer fmb;

    fmb.key = key;
    fmb.filePath = buffer_path;

    fmb.hFile = CreateFileA(buffer_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fmb.hFile == NULL) {
        std::filesystem::remove(buffer_path);
        throw std::runtime_error("could not create/open output buffer file");
    }

    fmb.hMapFile = CreateFileMappingA(fmb.hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (fmb.hMapFile == NULL) {
        CloseHandle(fmb.hFile);
        std::filesystem::remove(buffer_path);
        throw std::runtime_error("could not create file mapping object for output buffer file");
    }

    fmb.pMemory = MapViewOfFile(fmb.hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (fmb.pMemory == NULL) {
        CloseHandle(fmb.hMapFile);
        CloseHandle(fmb.hFile);
        std::filesystem::remove(buffer_path);
        throw std::runtime_error("failed to map output buffer file into memory");
    }

    fmb.ptr = reinterpret_cast<uint8_t*>(fmb.pMemory);
    fmb.ptr[0] = 0xFF;

    file_memory_buffers[key] = fmb;

    return fmb.key;
}

void RemoveOldFileMemoryBuffers()
{
    WF_LOG(LogLevel::LINFO, "deleting file memory buffer left from previous execution");

    for (const auto& entry : std::filesystem::directory_iterator(GetAppDataDir())) {
        if (entry.is_regular_file() && entry.path().extension() == ".dat") {
            std::filesystem::remove(entry.path());
            WF_LOG(LogLevel::LINFO, std::format("deleted file {}", entry.path().string()));
        }
    }
}

FileMemoryBuffer GetFileMemoryBuffer(uint16_t key)
{
    WF_LOG(LogLevel::LINFO, std::format("retrieving file memory buffer ({})", key));

    std::map<uint16_t, FileMemoryBuffer>::iterator it = file_memory_buffers.find(key);
    if (it == file_memory_buffers.end()) {
        throw std::runtime_error(std::format("could not find file memory buffer {}", key));
    }
    return file_memory_buffers[key];
}

void DeleteAllFileMemoryBuffers()
{
    WF_LOG(LogLevel::LINFO, "deleting all file memory buffers");

    std::vector<uint16_t> keys;

    for (const auto& pair : file_memory_buffers) {
        keys.push_back(pair.second.key);
    }

    for (const auto& key : keys) {
        DeleteFileMemoryBuffer(key);
    }
}

}