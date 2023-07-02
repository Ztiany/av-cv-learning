#ifndef OPENGL_BASIC_LOGKIT_H
#define OPENGL_BASIC_LOGKIT_H

#include <string>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

void openLog();

void logDebug(const char *message, ...);

void logInfo(const char *message, ...);

void logWarning(const char *message, ...);

void logError(const char *message, ...);

void closeLog();

#endif //OPENGL_BASIC_LOGKIT_H
