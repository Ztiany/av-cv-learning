package me.ztiany.androidav.opengl.jwopengl

import android.opengl.GLSurfaceView
import me.ztiany.androidav.databinding.OpenglActivityJavaMainBinding
import me.ztiany.lib.avbase.BaseActivity

class JavaWithOpenGLMainActivity : BaseActivity<OpenglActivityJavaMainBinding>() {

    private fun startCommon(clazz: Class<out GLSurfaceView.Renderer>) {
        JavaWithOpenGLCommonActivity.start(this, clazz)
    }

    fun drawBackground(view: android.view.View) {
        startCommon(BackgroundRenderer::class.java)
    }

    fun drawTriangle(view: android.view.View) {
        startCommon(TriangleRenderer::class.java)
    }

}