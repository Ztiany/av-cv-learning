package me.ztiany.androidav.opengl

import android.annotation.SuppressLint
import android.content.Context
import android.opengl.GLSurfaceView

@SuppressLint("ViewConstructor")
class TemplateGLSurfaceView(
    context: Context,
    renderer: Renderer,
    version: Int = 2,
) : GLSurfaceView(context) {

    init {
        setEGLContextClientVersion(version)
        setRenderer(renderer)
    }

}