#include "Sence.h"
#include <jni.h>

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_OGLESInterface_Init(JNIEnv *env, jobject thiz) {
    glClearColor(0.6F, 0.4F, 0.1F, 1.0F);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_OGLESInterface_OnViewportChanged(
        JNIEnv *env,
        jobject thiz,
        jint width,
        jint height
) {
    glViewport(0, 0, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwopengl_OGLESInterface_Render(JNIEnv *env, jobject thiz) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
