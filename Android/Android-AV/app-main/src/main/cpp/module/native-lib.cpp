#include <jni.h>
#include <string>
#include "common/log.h"

extern "C" {
#include "libavutil/avutil.h"
}

JavaVM *javaVM;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    xLogE("Native -------------------------------开始加载");
    xLogE("FFmpeg configuration");
    xLogE(avutil_configuration());
    xLogE("Native -------------------------------加载完成");
    return JNI_VERSION_1_4;
}
