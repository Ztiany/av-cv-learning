package me.ztiany.androidav.opengl.nwglsurv

import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class NativeRenderer : GLSurfaceView.Renderer {

    private val oglesInterface by lazy {
        OGLESInterface()
    }

    override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
        oglesInterface.Init()
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        oglesInterface.OnViewportChanged(width, height)
    }

    override fun onDrawFrame(gl: GL10) {
        oglesInterface.Render()
    }

}