#include "window.h"
#include "config.h"
#include "convert.h"
#include "displays.h"
#include "log.h"

#include <format>
#include <iostream>
#include <regex>

#include <windows.h>

namespace wallflow {

HWND hWnd;
NOTIFYICONDATA nid;
bool should_exit = false;

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

void ShowContextMenu(HWND hWnd)
{
    HMENU hMenu = CreatePopupMenu();

    int display_menu_offset = 1000;

    for (size_t i = 0; i < displays.size(); ++i) {
        std::string label = std::format("Refresh {}", displays[i].alias);
        AppendMenu(hMenu, MF_STRING, display_menu_offset + i, StringToWString(label).c_str());
    }

    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, TRAY_REFRESH_ALL, L"Refresh All");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(hMenu, MF_STRING, TRAY_OPEN_CONFIG, L"Open Config");
    AppendMenu(hMenu, MF_STRING, TRAY_OPEN_ALIASES, L"Open Aliases");

    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, TRAY_EXIT, L"Exit");

    // Get the mouse cursor's position
    POINT pt;
    GetCursorPos(&pt);

    // Display the context menu
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

    // Cleanup
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DISPLAYCHANGE:
        WF_LOG(LogLevel::LINFO, "display change detected");
        LoadDisplays();
        break;
    case WM_TRAY_ICON:
        switch (lParam) {
        case WM_RBUTTONUP:
            ShowContextMenu(hWnd);
            break;
        }

    case WM_COMMAND:
        if (LOWORD(wParam) >= TRAY_DISPLAY_OFFSET) {
            int display_index = LOWORD(wParam) - TRAY_DISPLAY_OFFSET;
            std::string message = std::format("refreshing display {}", displays[display_index].ToString());
            WF_LOG(LogLevel::LINFO, message);
            break;
        }

        switch (LOWORD(wParam)) {
        case TRAY_OPEN_CONFIG: {
            WF_LOG(LogLevel::LINFO, "opening config file " + GetConfigPath());
            HINSTANCE result = ShellExecuteA(NULL, NULL, GetConfigPath().c_str(), NULL, NULL, SW_SHOWNORMAL);
            break;
        }

        case TRAY_OPEN_ALIASES:
            WF_LOG(LogLevel::LINFO, "opening display alias file " + GetDisplayAliasPath());
            ShellExecuteA(NULL, "open", GetDisplayAliasPath().c_str(), NULL, NULL, SW_SHOWNORMAL);
            break;

        case TRAY_EXIT: // Handle Exit
            Shell_NotifyIcon(NIM_DELETE, &nid);
            DestroyWindow(hWnd);
            should_exit = true;
            break;
        case TRAY_REFRESH_ALL:
            WF_LOG(LogLevel::LINFO, "refreshing all displays");
            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void addTaskTrayIcon()
{
    HICON hIcon = (HICON)(LoadImage(NULL, L"icon.ico", IMAGE_ICON, 64, 64, LR_LOADFROMFILE | LR_SHARED));

    if (hIcon == NULL) {
        throw std::runtime_error("failed to load icon file");
    }

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 100;
    nid.uVersion = NOTIFYICON_VERSION;
    nid.uCallbackMessage = WM_TRAY_ICON;
    nid.hIcon = hIcon;
    wcscpy_s(nid.szTip, L"WallFlow");
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void InitWindow()
{
    hWnd = CreateWindow(L"Static", L"WallFlow", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr,
        GetModuleHandle(nullptr), nullptr);

    SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));

    addTaskTrayIcon();
}

}