package me.ztiany.androidav.opengl.jwopengl.painter

import android.opengl.GLES20
import android.opengl.Matrix
import me.ztiany.androidav.opengl.jwopengl.common.*

/**灵魂出鞘效果，注意：其接收来自相机的纹理。*/
class SoulFilter : BaseGLFilter() {

    private val glMVPMatrix = GLMVPMatrix()

    private lateinit var glFBO: GLFBOWithTexture

    /**用于修正相机的方向*/
    private var displayOrientation = 0

    /**矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /**纹理坐标*/
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateStandard())

    private var mixPercent = 0F
    private var scalePercent = 0F

    override fun createAndInitProgram(): GLProgram {
        val glProgram = GLProgram.fromAssets(
            "shader/vertex_mvp.glsl",
            "shader/fragment_soul_OES.glsl"
        )

        //vertex
        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")
        glProgram.activeUniform("uMVPModelMatrix")

        //fragment
        glProgram.activeUniform("scalePercent")
        glProgram.activeUniform("mixPercent")
        glProgram.activeUniform("uTexture")

        return glProgram
    }

    override fun setWorldSize(width: Int, height: Int) {
        val glTexture = generateFBOTexture(
            glProgram.uniformHandle("uTexture"),
            1,
            width,
            height
        )

        glFBO = generateFBOWithTexture(glTexture)
        glMVPMatrix.setWorldSize(width, height)
        adjustMatrix()
    }

    override fun setTextureAttribute(width: Int, height: Int, orientation: Int) {
        super.setTextureAttribute(width, height, orientation)
        this.displayOrientation = orientation
        glMVPMatrix.setModelSize(width, height)
        adjustMatrix()
    }

    private fun adjustMatrix() {
        glMVPMatrix.lookAtNormally()
        glMVPMatrix.adjustToOrthogonal()
        glMVPMatrix.combineMVP()
        //绕着 Z 轴旋转【因为 FBO 是倒着的】
        Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, -this.displayOrientation.toFloat(), 0F, 0F, 1F)
    }

    override fun onDrawFrame(sharedTexture: GLTexture): GLTexture {
        glFBO.use {
            GLES20.glViewport(0, 0, glMVPMatrix.getWorldWidth(), glMVPMatrix.getWorldHeight())
            doDraw(sharedTexture)
        }
        return glFBO.texture
    }

    private fun doDraw(sharedTexture: GLTexture) {
        glProgram.startDraw {
            clearColorBuffer()
            //vertex
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            //fragment
            uniformMatrix4fv("uMVPModelMatrix", glMVPMatrix.mvpMatrix)
            uniform1f("scalePercent", 1.0F + scalePercent)//[1, 2]
            uniform1f("mixPercent", 1.0F - mixPercent)//[1,0]
            updatePercent()
            //texture【将分享过来的纹理绘制到 FBO 的纹理上】
            sharedTexture.activeTexture(uniformHandle("uTexture"))
            //draw
            drawArraysStrip(4/*4 个顶点*/)
        }
    }

    private fun updatePercent() {
        scalePercent += 0.08F
        mixPercent += 0.08F
        if (scalePercent >= 1.0) {
            scalePercent = 0.0F
        }
        if (mixPercent >= 1.0) {
            mixPercent = 0.0F
        }
    }

}