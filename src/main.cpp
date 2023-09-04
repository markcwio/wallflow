#include "config.h"
#include "log.h"
#include "paths.h"
#include "window.h"

#include <iostream>

void initResources()
{
    WF_LOG(LogLevel::LINFO, "initialising resources");
    WF_START_TIMER("initResources()");
    wallflow::CreateAppDataDir();
    wallflow::LoadConfig();
    WF_END_TIMER("initResources()");
}

int main()
{
    try {
        initResources();
    } catch (const std::exception& ex) {
        WF_LOG(LogLevel::LFATAL, ex.what());
        return 1;
    }
    return 0;
}