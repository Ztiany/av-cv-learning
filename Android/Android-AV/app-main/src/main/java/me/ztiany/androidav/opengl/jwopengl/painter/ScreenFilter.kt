package me.ztiany.androidav.opengl.jwopengl.painter

import android.opengl.GLES20
import android.opengl.Matrix
import me.ztiany.androidav.opengl.jwopengl.common.*

/**灵魂出鞘效果，注意：其接收来自相机的纹理。*/
class ScreenFilter : BaseGLFilter() {

    private val glMVPMatrix = GLMVPMatrix()

    /**用于修正相机的方向*/
    private var displayOrientation = 0

    /**矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /**纹理坐标*/
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateAndroid())

    override fun createAndInitProgram(): GLProgram {
        val glProgram = GLProgram.fromAssets(
            "shader/vertex_base.glsl",
            "shader/fragment_texture.glsl"
        )

        //vertex
        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")
//        glProgram.activeUniform("uMVPModelMatrix")

        //fragment
        glProgram.activeUniform("uTexture")

        return glProgram
    }

    override fun setWorldSize(width: Int, height: Int) {
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
        //绕着 Z 轴旋转
        Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, -this.displayOrientation.toFloat(), 0F, 0F, 1F)
    }

    override fun onDrawFrame(sharedTexture: GLTexture): GLTexture {
        GLES20.glViewport(0, 0, glMVPMatrix.getWorldWidth(), glMVPMatrix.getWorldHeight())
        doDraw(sharedTexture)
        return sharedTexture
    }

    private fun doDraw(sharedTexture: GLTexture) {
        glProgram.startDraw {
            //vertex
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
//            uniformMatrix4fv("uMVPModelMatrix", glMVPMatrix.mvpMatrix)
            //fragment
            sharedTexture.activeTexture(uniformHandle("uTexture"))
            //draw
            drawArraysStrip(4/*4 个顶点*/)
        }
    }


}