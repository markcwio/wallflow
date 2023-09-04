/**
 * @file logging.h
 * @brief Logging and timer macros.
 */

#pragma once

#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

/**
 * @brief Enumeration representing different log levels.
 */
enum class LogLevel {
    LINFO, ///< Informational messages.
    LWARNING, ///< Warning messages.
    LERROR, ///< Error messages.
    LFATAL ///< Fatal messages.
};

#ifndef PROJECT_ROOT
#define PROJECT_ROOT ""
#endif

#ifdef ENABLE_LOGGING

/**
 * @brief Normalize file path by replacing backslashes with forward slashes.
 * @param path The file path to normalize.
 * @return The normalized file path.
 */
std::string normalizeFilePath(const char* path);

// Define a constant for the normalized project root path
extern const std::string normalizedProjectRoot;

/**
 * @brief Convert log level enum to string.
 * @param level The log level.
 * @return The log level as a string.
 */
std::string logLevelToString(LogLevel level);

/**
 * @brief Prefix macro that adds filename, date, and time to log entries.
 */
#define WF_PREFIX()                                                           \
    do {                                                                      \
        auto current_time = std::chrono::system_clock::now();                 \
        std::string file(normalizeFilePath(__FILE__));                        \
        size_t rootPos = file.find(normalizedProjectRoot);                    \
        if (rootPos != std::string::npos) {                                   \
            file = file.substr(rootPos + normalizedProjectRoot.size());       \
        }                                                                     \
        std::time_t now = std::chrono::system_clock::to_time_t(current_time); \
        std::cout << std::put_time(std::localtime(&now), "%F %T") << " ";     \
        std::cout << file << ":" << __LINE__ << " ";                          \
    } while (false)

/**
 * @brief Log macro that logs a message with a specified log level.
 * @param level The log level.
 * @param message The message to log.
 */
#define WF_LOG(level, message)                               \
    do {                                                     \
        WF_PREFIX();                                         \
        std::cout << "[" << logLevelToString(level) << "] "; \
        std::cout << message << std::endl;                   \
    } while (false)

/**
 * @brief Map to store timer start times.
 */
extern std::map<std::string, std::chrono::high_resolution_clock::time_point> timer_times;

/**
 * @brief Macro to start a timer with a given label.
 * @param label The label for the timer.
 */
#define WF_START_TIMER(label)                                           \
    do {                                                                \
        WF_PREFIX();                                                    \
        timer_times[label] = std::chrono::high_resolution_clock::now(); \
        std::cout << "[TIMER(" << label << ")] ";                       \
        std::cout << "STARTED" << std::endl;                            \
    } while (false)

/**
 * @brief Macro to end a timer with a given label.
 * @param label The label for the timer.
 */
#define WF_END_TIMER(label)                                                                                                   \
    do {                                                                                                                      \
        if (timer_times.find(label) != timer_times.end()) {                                                                   \
            WF_PREFIX();                                                                                                      \
            auto end_time = std::chrono::high_resolution_clock::now();                                                        \
            auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - timer_times[label]).count(); \
            std::cout << "[TIMER(" << label << ")] ";                                                                         \
            std::cout << "ENDED(" << std::format("{:.3f}", elapsed_time / 1e3) << "ms)" << std::endl;                         \
        }                                                                                                                     \
    } while (false)

/**
 * @brief Log macro that logs information about a pointer to an object.
 * @param obj The pointer to the object.
 */
#define WF_LOG_OBJ_PTR(obj)                                     \
    do {                                                        \
        WF_PREFIX();                                            \
        std::cout << "[OBJ*] " << obj->ToString() << std::endl; \
    } while (false)

/**
 * @brief Log macro that logs information about an object.
 * @param obj The object.
 */
#define WF_LOG_OBJ(obj)                                       \
    do {                                                      \
        WF_PREFIX();                                          \
        std::cout << "[OBJ] " << obj.ToString() << std::endl; \
    } while (false)

#else

// Define empty macros when logging is disabled
#define WF_LOG(level, message) \
    do {                       \
    } while (false)

#define WF_START_TIMER(label) \
    do {                      \
    } while (false)

#define WF_END_TIMER(label) \
    do {                    \
    } while (false)

#define WF_LOG_OBJ_PTR(obj) \
    do {                    \
    } while (false)

#define WF_LOG_OBJ(obj) \
    do {                \
    } while (false)

#endif