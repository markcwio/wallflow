#pragma once

#include <map>
#include <string>
#include <vector>

namespace wallflow {

extern std::map<std::string, int> repo_indexes;
extern std::map<std::string, std::vector<std::string>> repo_files;

void PopulateRepo(uint16_t width, uint16_t height);
void PopulateAllRepos();
std::string GetNextImage(uint16_t width, uint16_t height);

}