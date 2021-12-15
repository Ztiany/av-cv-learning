#include <jni.h>
#include <string>
#include "common/log.h"

extern "C" {
#include "libavutil/avutil.h"
}

JavaVM *javaVM;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    XLOGE("Native -------------------------------开始加载");
    XLOGE("FFmpeg configuration: %s", avutil_configuration());
    XLOGE("Native -------------------------------加载完成");
    return JNI_VERSION_1_4;
}
