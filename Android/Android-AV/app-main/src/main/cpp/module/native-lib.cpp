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
    long start = getNowMs();
    xLogE("Native -------------------------------开始加载");
    xLogE("FFmpeg configuration");
    xLogE(avutil_configuration());
    xLogE("Native -------------------------------加载完成, 耗时：%lld ms", (getNowMs() - start));
    return JNI_VERSION_1_4;
}
