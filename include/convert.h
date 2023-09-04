#pragma once

#include <string>

namespace wallflow {

std::wstring StringToWString(std::string& str);
std::string WStringToString(std::wstring& str);

}
