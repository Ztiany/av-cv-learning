#include "log.h"

#define LOG_LENGTH 1024

#define SAFE_TAG(TAG) TAG.empty()?DEFAULT_TAG:TAG.c_str()
#define LOGGER(LEVEL, TAG, ...) __android_log_print(LEVEL,TAG,__VA_ARGS__)

static void xLog(android_LogPriority level, std::string &tag, std::string &msg) {
    size_t len = msg.length();
    if (len > LOG_LENGTH) {
        for (int i = 0; i < len; i += LOG_LENGTH) {
            if (i + LOG_LENGTH < len) {
                LOGGER(level, SAFE_TAG(tag), "%s", msg.substr(i, i + LOG_LENGTH).c_str());
            } else {
                LOGGER(level, SAFE_TAG(tag), "%s", msg.substr(i, msg.length()).c_str());
            }
        }
    } else {
        LOGGER(level, SAFE_TAG(tag), "%s", msg.c_str());
    }
}

void xLogD(std::string &tag, std::string &msg) {
    xLog(ANDROID_LOG_DEBUG, tag, msg);
}

void xLogD(std::string &msg) {
    std::string tag;
    xLog(ANDROID_LOG_DEBUG, tag, msg);
}

void xLogD(const char *msg) {
    std::string str_msg(msg);
    xLogD(str_msg);
}

void xLogD(const char *tag, const char *msg) {
    std::string str_msg(msg);
    std::string str_tag(msg);
    xLogD(str_tag, str_msg);
}

void xLogE(std::string &msg) {
    std::string tag;
    xLog(ANDROID_LOG_ERROR, tag, msg);
}

void xLogE(std::string &tag, std::string &msg) {
    xLog(ANDROID_LOG_ERROR, tag, msg);
}

void xLogE(const char *msg) {
    std::string str_msg(msg);
    xLogE(str_msg);
}

void xLogE(const char *tag, const char *msg) {
    std::string str_msg(msg);
    std::string str_tag(msg);
    xLogE(str_tag, str_msg);
}

void xLogI(std::string &msg) {
    std::string tag;
    xLog(ANDROID_LOG_INFO, tag, msg);
}

void xLogI(std::string &tag, std::string &msg) {
    xLog(ANDROID_LOG_INFO, tag, msg);
}

void xLogI(const char *msg) {
    std::string str_msg(msg);
    xLogI(str_msg);
}

void xLogI(const char *tag, const char *msg) {
    std::string str_msg(msg);
    std::string str_tag(msg);
    xLogI(str_tag, str_msg);
}