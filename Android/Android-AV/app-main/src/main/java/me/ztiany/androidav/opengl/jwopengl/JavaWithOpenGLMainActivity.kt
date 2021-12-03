package me.ztiany.androidav.opengl.jwopengl

import android.opengl.GLSurfaceView
import android.view.View
import me.ztiany.androidav.databinding.OpenglActivityJavaMainBinding
import me.ztiany.androidav.opengl.jwopengl.renderer.BackgroundRenderer
import me.ztiany.androidav.opengl.jwopengl.renderer.GradualTriangleRenderer
import me.ztiany.androidav.opengl.jwopengl.renderer.TriangleRenderer
import me.ztiany.lib.avbase.BaseActivity

class JavaWithOpenGLMainActivity : BaseActivity<OpenglActivityJavaMainBinding>() {

    private fun startCommon(clazz: Class<out GLSurfaceView.Renderer>) {
        JavaWithOpenGLCommonActivity.start(this, clazz)
    }

    fun drawBackground(view: View) {
        startCommon(BackgroundRenderer::class.java)
    }

    fun drawTriangle(view: View) {
        startCommon(TriangleRenderer::class.java)
    }

    fun drawRect(view: View) {
        startCommon(GradualTriangleRenderer::class.java)
    }

}