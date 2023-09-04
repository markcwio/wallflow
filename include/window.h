#pragma once

#include <string>

#include <Windows.h>

namespace wallflow {

struct WindowsError {
    unsigned long code;
    std::string message;
};

WindowsError GetLastWindowsError();
void PrintWindowsError(WindowsError& error);

}