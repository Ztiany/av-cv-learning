#include <jni.h>
#include <vector>
#include <log.h>
#include "common/GLRenderer.h"
#include "sample/BackgroundRenderer.hpp"

long createNativeRenderer(jint type) {
    GLRenderer *result = nullptr;
    if (type == BackgroundRenderer::TYPE) {
        auto *renderer = new BackgroundRenderer();
        result = renderer;
    }
    return reinterpret_cast<long>(result);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_createNativeRenderer(JNIEnv *env, jobject thiz, jint type) {
    LOGD("createNativeRenderer");
    return createNativeRenderer(type);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_onSurfaceCreated(JNIEnv *env, jobject thiz, jlong handle) {
    LOGD("onSurfaceCreated");
    auto *renderer = reinterpret_cast<GLRenderer *>(handle);
    renderer->onSurfaceCreated();
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_onViewportChanged(JNIEnv *env, jobject thiz, jlong handle, jint width, jint height) {
    LOGD("onSurfaceCreated, width = %d, height = %d", width, height);
    auto *renderer = reinterpret_cast<GLRenderer *>(handle);
    renderer->onSurfaceChanged(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_onDrawFrame(JNIEnv *env, jobject thiz, jlong handle) {
    auto *renderer = reinterpret_cast<GLRenderer *>(handle);
    renderer->onDrawFrame(nullptr);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_NativeRenderer_onSurfaceDestroy(JNIEnv *env, jobject thiz, jlong handle) {
    LOGD("onSurfaceDestroy");
    auto *renderer = reinterpret_cast<GLRenderer *>(handle);
    renderer->onSurfaceDestroy();
    delete renderer;
}
