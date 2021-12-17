#include <jni.h>
#include <string>
#include "utils/Log.h"
#include "pusher/RtmpPusher.h"

JavaVM *javaVM;
static RtmpPusher *rtmpPusher;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    LOGE("Native 加载完成");
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_init(JNIEnv *env, jobject thiz) {
    //check if already started.
    if (rtmpPusher) {
        return;
    }
    rtmpPusher = new RtmpPusher();
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_release(JNIEnv *env, jobject thiz) {
    if (rtmpPusher) {
        rtmpPusher->stop();
    }
    rtmpPusher = nullptr;
}
extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_initVideoCodec(JNIEnv *env, jobject thiz, jint width,
                                                         jint height, jint fps, jint bitrate) {
    // TODO: implement initVideoCodec()
}
extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_initAudioCodec(JNIEnv *env, jobject thiz,
                                                         jint sample_rate, jint channels) {
    // TODO: implement initAudioCodec()
}
extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_sendVideoPacket(JNIEnv *env, jobject thiz,
                                                          jbyteArray data, jint video_type) {
    // TODO: implement sendVideoPacket()
}
extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_start(JNIEnv *env, jobject thiz, jstring url) {
    if (!rtmpPusher) {
        return;
    }
    /*if(rtmpPusher->isPushing()){
        return;
    }*/

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
Java_me_ztiany_rtmp_livevideo_MediaPusher_sendAudioPacket(
        JNIEnv *env,
        jobject thiz,
        jbyteArray data,
        jint audio_type
) {

}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_rtmp_livevideo_MediaPusher_stop(JNIEnv *env, jobject thiz) {
    // TODO: implement stop()
}