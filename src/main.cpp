#include "config.h"
#include "displays.h"
#include "log.h"
#include "mem.h"
#include "paths.h"
#include "repo.h"
#include "wallpapers.h"
#include "window.h"

#include <csignal>
#include <iostream>

#include <windows.h>

void cleanup()
{
    WF_START_TIMER("cleanup()");
    try {
        wallflow::DeleteAllFileMemoryBuffers();
        wallflow::should_exit = true;
    } catch (const std::exception& ex) {
        WF_LOG(LogLevel::LERROR, ex.what());
    }
    WF_END_TIMER("cleanup()");
}

std::atomic<std::chrono::steady_clock::time_point> last_run_at;

void cycleWallpapers()
{
    while (!wallflow::should_exit) {
        auto current_time = std::chrono::steady_clock::now();
        auto interval = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_run_at.load()).count();

        if (interval >= wallflow::config->cycleSpeed) {
            WF_LOG(LogLevel::LINFO, "scheduled wallpaper cycle");
            wallflow::CycleAllDisplays();
            last_run_at = current_time;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void initResources()
{
    WF_LOG(LogLevel::LINFO, "initialising resources");
    WF_START_TIMER("initResources()");
    wallflow::CreateAppDataDir();
    wallflow::LoadConfig();
    wallflow::LoadDisplays();
    wallflow::PopulateAllRepos();
    wallflow::RemoveOldFileMemoryBuffers();
    wallflow::InitWindow();
    WF_END_TIMER("initResources()");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    try {

#ifdef ENABLE_LOGGING
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
#endif

        initResources();

        std::thread cycleWallpapersThread(cycleWallpapers);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        cleanup();
        cycleWallpapersThread.join();

#ifdef ENABLE_LOGGING
        FreeConsole();
#endif

        return static_cast<int>(msg.wParam);

    } catch (const std::exception& ex) {
        WF_LOG(LogLevel::LFATAL, ex.what());
        return 1;
    }
    return 0;
}