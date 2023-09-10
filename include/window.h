#pragma once

#include <string>

#include <windows.h>

#define WM_TRAY_ICON (WM_USER + 1)

#define TRAY_EXIT 1
#define TRAY_OPEN_CONFIG 2
#define TRAY_OPEN_ALIASES 3
#define TRAY_CYCLE_ALL 4
#define TRAY_CHANGE_WALLPAPER_DIR 5
#define TRAY_TOGGLE_SHUFFLE 6
#define TRAY_SHOW_CYCLE_INTERVAL 7
#define TRAY_SAVE_CYCLE_INTERVAL 8

#define TRAY_CYCLE_DISPLAY_OFFSET 1000

namespace wallflow {

extern HWND hWnd;
extern NOTIFYICONDATA nid;
extern bool should_exit;

struct WindowsError {
    unsigned long code;
    std::string message;
};

WindowsError GetLastWindowsError();
void InitWindow();

}