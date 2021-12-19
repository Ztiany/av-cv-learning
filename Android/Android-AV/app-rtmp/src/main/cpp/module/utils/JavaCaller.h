#ifndef ANDROID_AV_JAVACALLER_H
#define ANDROID_AV_JAVACALLER_H

#include <jni.h>
#include "../utils/Log.h"

enum Thread {
    Child,
    Main
};

class JavaCaller {

private:
    _JavaVM *jvm = nullptr;
    JNIEnv *mainEnv = nullptr;
    jobject handle;

    jmethodID methodOnInitResult = nullptr;

    void findJavaMethods() {
        jclass rtmpPusherClass = mainEnv->GetObjectClass(handle);
        if (!rtmpPusherClass) {
            LOGE("get jclass wrong");
        }
        methodOnInitResult = mainEnv->GetMethodID(rtmpPusherClass, "onInitResult", "(Z)V");
    }

public:
    JavaCaller(_JavaVM *javaVM, JNIEnv *jniEnv, jobject *obj) : jvm(javaVM), mainEnv(jniEnv) {
        handle = jniEnv->NewGlobalRef(*obj);
        findJavaMethods();
    }

    ~JavaCaller() {
        jvm = nullptr;
        mainEnv->DeleteGlobalRef(handle);
        mainEnv = nullptr;
    }

    void notifyInitResult(bool succeeded, int type) {
        jboolean result = succeeded ? JNI_TRUE : JNI_FALSE;
        if (type == Main) {
            mainEnv->CallVoidMethod(handle, methodOnInitResult, result);
        } else {
            JNIEnv *jniEnv;
            if (jvm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
                LOGE("call onCallTimeInfo worng");
                return;
            }
            jniEnv->CallVoidMethod(handle, methodOnInitResult, result);
            jvm->DetachCurrentThread();
        }
    }

};

#endif //ANDROID_AV_JAVACALLER_H
