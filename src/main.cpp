#include "config.h"
#include "displays.h"
#include "log.h"
#include "paths.h"
#include "window.h"

#include <iostream>

#include <windows.h>

void initResources()
{
    WF_LOG(LogLevel::LINFO, "initialising resources");
    WF_START_TIMER("initResources()");
    wallflow::CreateAppDataDir();
    wallflow::LoadConfig();
    wallflow::LoadDisplays();
    WF_END_TIMER("initResources()");
}

int main()
{

    try {
        initResources();
        return 0;

    } catch (const std::exception& ex) {
        WF_LOG(LogLevel::LFATAL, ex.what());
        return 1;
    }
    return 0;
}