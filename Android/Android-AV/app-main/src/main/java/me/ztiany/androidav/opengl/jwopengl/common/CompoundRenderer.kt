package me.ztiany.androidav.opengl.jwopengl.common

import android.opengl.GLSurfaceView
import java.util.concurrent.CopyOnWriteArrayList
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class CompoundRenderer(defaultPainter: GLPainter? = null) : GLSurfaceView.Renderer {

    private val painterList = CopyOnWriteArrayList<GLPainter>()

    init {
        if (defaultPainter != null) {
            addPainter(defaultPainter)
        }
    }

    fun addPainter(painter: GLPainter) {
        painterList.add(painter)
    }

    fun removePainter(painter: GLPainter) {
        painterList.remove(painter)
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        painterList.forEach {
            it.onSurfaceCreated()
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        painterList.forEach {
            it.onSurfaceChanged(width, height)
        }
    }

    override fun onDrawFrame(gl: GL10?) {
        painterList.forEach {
            it.onDrawFrame()
        }
    }

}