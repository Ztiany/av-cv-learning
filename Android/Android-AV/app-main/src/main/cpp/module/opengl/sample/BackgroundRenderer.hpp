#ifndef ANDROID_AV_BACKGROUNDRENDERER_HPP
#define ANDROID_AV_BACKGROUNDRENDERER_HPP

#include "../common/GLRenderer.h"

class BackgroundRenderer : public GLRenderer {

public:
    static const int TYPE = 0;

    void onSurfaceCreated() override {
        /*
         * 理解 glClearColor：这里 gl 是 OpenGL API 统一的前缀，ClearColor 是一个名词，表示清屏颜色。
         */
        glClearColor(0.6F, 0.4F, 0.1F, 1.0F);
    }

    void onSurfaceChanged(int width, int height) override {
        glViewport(0, 0, width, height);
    }

    void onDrawFrame(void *data) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void onSurfaceDestroy() override {

    }

};

#endif //ANDROID_AV_BACKGROUNDRENDERER_HPP
