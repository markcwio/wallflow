#pragma once

#include <string>

#include <windows.h>

#define WM_TRAY_ICON (WM_USER + 1)
#define TRAY_EXIT 1
#define TRAY_OPEN_CONFIG 2
#define TRAY_OPEN_ALIASES 3
#define TRAY_REFRESH_ALL 4
#define TRAY_DISPLAY_OFFSET 1000

namespace wallflow {

extern HWND hWnd;
extern NOTIFYICONDATA nid;
extern bool should_exit;

struct WindowsError {
    unsigned long code;
    std::string message;
};

WindowsError GetLastWindowsError();
void PrintWindowsError(WindowsError& error);
void InitWindow();

}