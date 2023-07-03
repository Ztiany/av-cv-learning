#ifndef ANDROID_AV_LOG_H
#define ANDROID_AV_LOG_H

#include <string>
#include <android/log.h>

#define DEFAULT_TAG "Native"

void xLog(int priority, const char *tag, const char *format, ...);

void xLogD(const char *tag, const char *format, ...);

void xLogI(const char *tag, const char *format, ...);

void xLogW(const char *tag, const char *format, ...);

void xLogE(const char *tag, const char *format, ...);

#define LOGD(...) xLogD(DEFAULT_TAG,__VA_ARGS__)
#define LOGI(...) xLogI(DEFAULT_TAG,__VA_ARGS__)
#define LOGW(...) xLogW(DEFAULT_TAG,__VA_ARGS__)
#define LOGE(...) xLogE(DEFAULT_TAG,__VA_ARGS__)

#define LOGD_TAG(TAG, ...) xLogD(TAG,__VA_ARGS__)
#define LOGI_TAG(TAG, ...) xLogI(TAG,__VA_ARGS__)
#define LOGW_TAG(TAG, ...) xLogW(TAG,__VA_ARGS__)
#define LOGE_TAG(TAG, ...) xLogE(TAG,__VA_ARGS__)

#endif //ANDROID_AV_LOG_H
