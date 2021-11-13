#include "Sence.h"
#include <jni.h>

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwglsurv_OGLESInterface_Init(JNIEnv *env, jobject thiz) {
    glClearColor(0.6f, 0.4f, 0.1f, 1.0f);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwglsurv_OGLESInterface_OnViewportChanged(
        JNIEnv *env,
        jobject thiz,
        jint width,
        jint height
) {
    glViewport(0, 0, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_ztiany_androidav_opengl_nwglsurv_OGLESInterface_Render(JNIEnv *env, jobject thiz) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void test() {
    GLuint gLuint = glCreateShader(GL_VERTEX_SHADER);
}
