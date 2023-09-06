#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace wallflow {

struct Display {
    std::string id;
    std::string alias;
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
    std::string repoKey;
    std::string ToString();
};

extern std::vector<Display> displays;

void LoadDisplays();

}