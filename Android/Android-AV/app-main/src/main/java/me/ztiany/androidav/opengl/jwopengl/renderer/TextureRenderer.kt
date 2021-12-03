package me.ztiany.androidav.opengl.jwopengl.renderer

import android.opengl.GLES20
import android.opengl.GLSurfaceView
import me.ztiany.androidav.opengl.jwopengl.common.GLProgram
import me.ztiany.androidav.opengl.jwopengl.common.generateVBOBuffer
import me.ztiany.androidav.opengl.jwopengl.common.newRectVertex3Coordination
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class TextureRenderer : GLSurfaceView.Renderer {

    private lateinit var program: GLProgram

    /*四个点的颜色*/
    private val vertexColor = floatArrayOf(
        1.0F, 0.0F, 0.0F, 1.0F,
        0.0F, 1.0F, 0.0F, 1.0F,
        0.0F, 0.0F, 1.0F, 1.0F,
        1.0F, 1.0F, 1.0F, 1.0F,
    )

    /*矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(
        newRectVertex3Coordination().map {
            it * 0.8F
        }.toFloatArray()
    )

    /*矩形的坐标颜色*/
    private val colorBuffer = generateVBOBuffer(vertexColor)

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        program = GLProgram.fromAssets("shader/base_vert.glsl", "shader/coloring_fragment.glsl")
        program.activeAttribute("aPosition")
        program.activeAttribute("aColor")
        program.setColor(0.7F, 0.5F, 0.5F, 1.0F)
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
        program.setSize(width, height)
    }

    override fun onDrawFrame(gl: GL10?) {
        program.startDraw {
            clearColorBuffer()
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aColor", 4, colorBuffer)
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4/*4 个点*/)
        }
    }
}