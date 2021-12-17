#ifndef ANDROID_AV_JAVACALLER_H
#define ANDROID_AV_JAVACALLER_H

#include <jni.h>

enum Thread {
    Child,
    Main
};

class JavaCaller {

private:
    _JavaVM *jvm = nullptr;
    JNIEnv *mainEnv = nullptr;
    jobject handle;

public:
    JavaCaller(_JavaVM *javaVM, JNIEnv *jniEnv, jobject *obj) : jvm(javaVM), mainEnv(jniEnv) {
        handle = jniEnv->NewGlobalRef(*obj);
    }

    ~JavaCaller() {
        jvm = nullptr;
        mainEnv->DeleteGlobalRef(handle);
        mainEnv = nullptr;
    }

};

#endif //ANDROID_AV_JAVACALLER_H
