package me.ztiany.androidav.opengl.jwopengl.recorder.filter

import android.opengl.GLES20
import android.opengl.Matrix
import me.ztiany.androidav.opengl.jwopengl.gles2.*
import timber.log.Timber

/**渲染的是 FBO 中的纹理，使用标准的坐标系*/
class FoundationScreenFilter : BaseGLFilter() {

    private val glMVPMatrix = GLMVPMatrix()

    /**用于修正相机的方向*/
    private var displayOrientation = 0

    /**用于修正相机的镜像【前置】*/
    private var isFront = false

    /**矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /**纹理坐标*/
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateStandard())

    override fun createAndInitProgram(): GLProgram {
        Timber.d("createAndInitProgram() called")

        val glProgram = GLProgram.fromAssets(
            "shader/vertex_mvp.glsl",
            "shader/fragment_texture.glsl"
        )

        //vertex
        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")
        glProgram.activeUniform("uMVPModelMatrix")

        //fragment
        glProgram.activeUniform("uTexture")

        return glProgram
    }

    override fun setWorldSize(width: Int, height: Int) {
        glMVPMatrix.setWorldSize(width, height)
        adjustMatrix()
    }

    override fun setTextureAttribute(attribute: TextureAttribute) {
        this.displayOrientation = attribute.orientation
        this.isFront = attribute.isFront
        if ((displayOrientation / 90).mod(2) == 1) {//竖屏
            glMVPMatrix.setModelSize(attribute.height, attribute.width)
        } else {//横屏
            glMVPMatrix.setModelSize(attribute.width, attribute.height)
        }
        adjustMatrix()
    }

    private fun adjustMatrix() {
        glMVPMatrix.resetToIdentity(glMVPMatrix.mvpMatrix)
        glMVPMatrix.lookAtNormally()
        glMVPMatrix.adjustToOrthogonal()
        glMVPMatrix.combineMVP()
        //绕着 Z 轴旋转
        if (!isFront) {
            //后摄，一般情况下相机的画面被逆时针转了 90 度，这是这里也将顶点坐标转同样的角度。
            //注意【顶点是先插值，然后我们利用矩阵再将顶点修正到正确的采样进行位置】。
            Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, -this.displayOrientation.toFloat(), 0F, 0F, 1F)
        } else {
            Matrix.scaleM(glMVPMatrix.mvpMatrix, 0, -1F, 1F, 1F)
            Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, this.displayOrientation.toFloat(), 0F, 0F, 1F)
        }
    }

    override fun doDraw(sharedTexture: GLTexture): GLTexture {
        GLES20.glViewport(0, 0, glMVPMatrix.getWorldWidth(), glMVPMatrix.getWorldHeight())
        glProgram.startDraw {
            //clear
            clearColorBuffer()
            //vertex
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            uniformMatrix4fv("uMVPModelMatrix", glMVPMatrix.mvpMatrix)
            //fragment
            sharedTexture.activeTexture(uniformHandle("uTexture"))
            //draw
            drawArraysStrip(4/*4 个顶点*/)
        }
        return sharedTexture
    }

}