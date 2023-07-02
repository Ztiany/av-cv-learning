#include "LogKit.h"
#include <Windows.h>
#include <cstdio>
#include <iostream>
#include <cstdarg>

#ifdef OPEN_LOG

void openLog() {
    // 如果是在 Win32 GUI 应用程序中使用 printf 函数或其他标准库输出函数，需要先创建控制台窗口，才能在控制台窗口中看到输出。
    // 可以通过调用 AllocConsole 函数来创建控制台窗口，然后使用标准库输出函数输出信息。需要注意的是，如果在 Visual Studio
    // 中运行程序，控制台窗口默认是不会显示的，需要手动打开。
    AllocConsole();
    //将标准输出重定向到控制台窗口
    FILE *pFile;
    freopen_s(&pFile, "CONOUT$", "w", stdout);
}

static void log(LogLevel level, const char *format, ...) {
    // 获取当前时间
    time_t t = std::time(nullptr);
    char timeStr[128];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

    // 根据日志级别选择输出前缀
    const char *prefix = "";
    switch (level) {
        case LogLevel::Debug:
            prefix = "[DEBUG]";
            break;
        case LogLevel::Info:
            prefix = "[INFO]";
            break;
        case LogLevel::Warning:
            prefix = "[WARNING]";
            break;
        case LogLevel::Error:
            prefix = "[ERROR]";
            break;
    }

    // 输出日志
    std::printf("%s %s ", timeStr, prefix);
    std::va_list args;
            va_start(args, format);
    std::vprintf(format, args);
            va_end(args);
    std::printf("\n");
}

void logDebug(const char *message, ...) {
    std::va_list args;
            va_start(args, message);
    log(LogLevel::Debug, message, args);
            va_end(args);
}

void logInfo(const char *message, ...) {
    std::va_list args;
            va_start(args, message);
    log(LogLevel::Info, message, args);
            va_end(args);

}

void logWarning(const char *message, ...) {
    std::va_list args;
            va_start(args, message);
    log(LogLevel::Warning, message, args);
            va_end(args);
}

void logError(const char *message, ...) {
    std::va_list args;
            va_start(args, message);
    log(LogLevel::Error, message, args);
            va_end(args);
}

void closeLog() {
    FreeConsole();
}

#elif
void openLog(){}
void logDebug(const std::string &message){}
void logInfo(const std::string &message){}
void logWarning(const std::string &message){}
void logError(const std::string &message){}
void closeLog() {}
#endif