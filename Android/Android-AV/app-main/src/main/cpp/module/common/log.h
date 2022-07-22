#ifndef ANDROID_AV_LOG_H
#define ANDROID_AV_LOG_H

#include <string>
#include <android/log.h>

#define DEFAULT_TAG "Native"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,DEFAULT_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,DEFAULT_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,DEFAULT_TAG,__VA_ARGS__)

#define LOGD_TAG(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGI_TAG(TAG, ...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGE_TAG(TAG, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

void xLogD(std::string &msg);

void xLogD(std::string &tag, std::string &msg);

void xLogD(const char *msg);

void xLogD(const char *tag, const char *msg);

void xLogE(std::string &msg);

void xLogE(std::string &tag, std::string &msg);

void xLogE(const char *msg);

void xLogE(const char *tag, const char *msg);

void xLogI(std::string &msg);

void xLogI(std::string &tag, std::string &msg);

void xLogI(const char *msg);

void xLogI(const char *tag, const char *msg);

#endif //ANDROID_AV_LOG_H
