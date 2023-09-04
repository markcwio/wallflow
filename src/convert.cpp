#include "convert.h"

#include <codecvt>

std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_converter;

namespace wallflow {

std::wstring StringToWString(std::string& str)
{
    return utf8_converter.from_bytes(str);
}

std::string WStringToString(std::wstring& wstr)
{
    return utf8_converter.to_bytes(wstr);
}

}