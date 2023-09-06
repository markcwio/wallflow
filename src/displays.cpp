#include "displays.h"
#include "config.h"
#include "convert.h"
#include "log.h"
#include "paths.h"

#include <format>
#include <fstream>

#include <windows.h>

namespace wallflow {

std::vector<Display> displays;
std::vector<Display> tmp_displays;

std::string Display::ToString()
{
    return std::format(
        "Display(id={},alias={},x={},y={},width={},height={})",
        id,
        alias,
        x,
        y,
        width,
        height);
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    MONITORINFOEX monitor_info;
    monitor_info.cbSize = sizeof(MONITORINFOEX);

    if (GetMonitorInfo(hMonitor, &monitor_info)) {

        Display display;

        std::wstring wid;
        for (int i = 0; i < 32 && monitor_info.szDevice[i] != L'\0'; ++i) {
            wid += monitor_info.szDevice[i];
        }

        RECT rect = monitor_info.rcMonitor;

        display.x = rect.left;
        display.y = rect.top;

        display.width = rect.right - rect.left;
        display.height = rect.bottom - rect.top;

        std::string repo_key = std::format("{}x{}", display.width, display.height);
        display.id = std::format("{}:{}", WStringToString(wid), repo_key);
        display.repoKey = repo_key;
        display.alias = GetOrCreateAlias(display.id, repo_key);

        tmp_displays.push_back(display);
    }

    return TRUE;
}

void correctDisplayOffsetsAndStore()
{
    int min_x = 0;
    int min_y = 0;

    for (Display display : tmp_displays) {
        if (display.x < min_x) {
            min_x = display.x;
        }
        if (display.y < min_y) {
            min_y = display.y;
        }
    }

    for (Display display : tmp_displays) {
        display.x = display.x - min_x;
        display.y = display.y - min_y;
        displays.push_back(display);
    }
}

std::mutex load_monitors_mtx;

void LoadDisplays()
{
    std::lock_guard<std::mutex> lock(load_monitors_mtx);

    tmp_displays.clear();

    WF_START_TIMER("LoadDisplays()");
    if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0)) {
        throw std::runtime_error("failed to get displays");
    }

    displays.clear();
    correctDisplayOffsetsAndStore();

    for (Display display : displays) {
        WF_LOG_OBJ(display);
    }

    WF_END_TIMER("LoadDisplays()");
}

}