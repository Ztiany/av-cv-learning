package me.ztiany.androidav.opengl.jwopengl.painter

import android.opengl.GLES20
import me.ztiany.androidav.opengl.jwopengl.common.GLPainter
import me.ztiany.androidav.opengl.jwopengl.common.GLProgram
import me.ztiany.androidav.opengl.jwopengl.common.generateVBOBuffer
import me.ztiany.androidav.opengl.jwopengl.common.newVertexCoordinateFull3

class GradualRectanglePainter : GLPainter {

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
        newVertexCoordinateFull3().map {
            it * 0.8F
        }.toFloatArray()
    )

    /*矩形的坐标颜色*/
    private val colorBuffer = generateVBOBuffer(vertexColor)

    override fun onSurfaceCreated() {
        program = GLProgram.fromAssets("shader/vertex_base.glsl", "shader/fragment_coloring.glsl")
        program.activeAttribute("aPosition")
        program.activeAttribute("aColor")
        program.setColor(0.7F, 0.5F, 0.5F, 1.0F)
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
    }

    override fun onDrawFrame() {
        program.startDraw {
            clearColorBuffer()
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aColor", 4, colorBuffer)
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4/*4 个点*/)
        }
    }

    override fun onSurfaceDestroy() {
    }

}