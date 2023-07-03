#include <jni.h>
#include <string>
#include "common/log.h"
#include "common/times.h"

extern "C" {
#include "libavutil/avutil.h"
}

JavaVM *javaVM;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    LOGE("Native -------------------------------开始加载");
    LOGE("FFmpeg configuration: %s", avutil_configuration());
    LOGE("Native -------------------------------加载完成");
    return JNI_VERSION_1_4;
}
