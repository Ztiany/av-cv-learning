package me.ztiany.androidav.opengl.java

import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class DemoRenderer : GLSurfaceView.Renderer {

    override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
        //设置当前颜色状态【OpenGL 是一个状态机】
        gl.glClearColor(0.1F, 0.4F, 0.6F, 1.0F)
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        //设置视口，Viewport 可以理解为 OpenGL 的画布。
        gl.glViewport(0, 0, width, height)
    }

    override fun onDrawFrame(gl: GL10) {
        //用 glClearColor 设置的颜色来擦除颜色缓冲区。【除了颜色缓冲区，还有 GL_DEPTH_BUFFER_BIT（深度） 和 GL_STENCIL_BUFFER_BIT（蒙版）】
        gl.glClear(GL10.GL_DEPTH_BUFFER_BIT)
    }

}