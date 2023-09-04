#pragma once

#include <map>
#include <string>
#include <vector>

namespace wallflow {

extern std::map<std::string, int> repo_indexes;
extern std::map<std::string, std::vector<std::string>> repo_files;

void PopulateRepo(std::string key);

}