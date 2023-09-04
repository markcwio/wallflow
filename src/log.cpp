#include "log.h"

std::string normalizeFilePath(const char* path)
{
    std::string normalizedPath(path);
    std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
    return normalizedPath;
}

std::string logLevelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::LINFO:
        return "INFO";
    case LogLevel::LWARNING:
        return "WARNING";
    case LogLevel::LERROR:
        return "ERROR";
    case LogLevel::LFATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

const std::string normalizedProjectRoot = normalizeFilePath(PROJECT_ROOT) + "/";
std::map<std::string, std::chrono::high_resolution_clock::time_point> timer_times;