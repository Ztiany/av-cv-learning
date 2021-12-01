#include <jni.h>
#include <string>
#include <Log.h>
#include <RtmpPusher.h>

JavaVM *javaVM;
static RtmpPusher *rtmpPusher;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    LOGE("Native 加载完成");
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_initPusher(JNIEnv *env, jobject thiz, jstring url) {
    //check if already started.
    if (rtmpPusher) {
        return;
    }
    rtmpPusher = new RtmpPusher();

    //convert the url
    const char *path = env->GetStringUTFChars(url, nullptr);
    char *address = new char[strlen(path) + 1];
    strcpy(address, path);
    env->ReleaseStringUTFChars(url, path);

    //start the pusher
    rtmpPusher->start(address);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_releasePusher(JNIEnv *env, jobject thiz) {
    if (rtmpPusher) {
        rtmpPusher->stop();
    }
    rtmpPusher = nullptr;
}