#ifndef ANDROID_AV_LOG_H
#define ANDROID_AV_LOG_H

#include <android/log.h>

#define XLOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"Native",__VA_ARGS__)
#define XLOGI(...) __android_log_print(ANDROID_LOG_INFO,"Native",__VA_ARGS__)
#define XLOGE(...) __android_log_print(ANDROID_LOG_ERROR,"Native",__VA_ARGS__)

/*
Other Platforms:
#define XLOGD(...) printf("XPlay\n",__VA_ARGS__)
#define XLOGI(...) printf("XPlay\n",__VA_ARGS__)
#define XLOGE(...) printf("XPlay\n",__VA_ARGS__)
 */

#endif //ANDROID_AV_LOG_H
