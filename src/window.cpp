#include "window.h"
#include "convert.h"

#include <format>
#include <iostream>
#include <regex>

namespace wallflow {

WindowsError GetLastWindowsError()
{
    WindowsError windows_error;

    DWORD error_code = GetLastError();
    windows_error.code = error_code;

    LPVOID error_msg;

    DWORD result = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        error_code,
        0, // Default language
        (LPWSTR)&error_msg,
        0,
        NULL);

    if (result != 0) {
        std::wstring wstr((LPWSTR)error_msg);
        windows_error.message = WStringToString(wstr);
    }

    return windows_error;
}

void PrintWindowsError(WindowsError& error)
{
    std::string message = error.message;

    std::regex pattern("^[\\s\\n]+|[\\s\\n]+$");

    message = std::regex_replace(message, pattern, "");

    std::cout << std::format("WindowsError(code={}, message={})", error.code, message) << std::endl;
}

}