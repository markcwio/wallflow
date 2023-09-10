#pragma once

#include <cstdint>
#include <string>

#define NOMINMAX

#include <windows.h>

namespace wallflow {

struct FileMemoryBuffer {
    uint16_t key;
    uint8_t* ptr;
    size_t size;
    HANDLE hFile;
    HANDLE hMapFile;
    LPVOID pMemory;
    std::string filePath;
};

void DeleteFileMemoryBuffer(uint16_t key);
FileMemoryBuffer GetFileMemoryBuffer(uint16_t key);
uint16_t CreateFileMemoryBuffer(size_t size);
void RemoveOldFileMemoryBuffers();
void DeleteAllFileMemoryBuffers();

}