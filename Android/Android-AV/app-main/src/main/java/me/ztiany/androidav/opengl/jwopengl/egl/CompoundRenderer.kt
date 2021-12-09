package me.ztiany.androidav.opengl.jwopengl.egl

import me.ztiany.androidav.opengl.jwopengl.common.GLPainter
import java.util.concurrent.CopyOnWriteArrayList

class CompoundRenderer(defaultPainter: GLPainter? = null) : GLRenderer {

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

    override fun onSurfaceCreated() {
        painterList.forEach {
            it.onSurfaceCreated()
        }
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        painterList.forEach {
            it.onSurfaceChanged(width, height)
        }
    }

    override fun onDrawFrame() {
        painterList.forEach {
            it.onDrawFrame()
        }
    }

    override fun onSurfaceDestroy() {
        painterList.forEach {
            it.onSurfaceDestroy()
        }
    }

}