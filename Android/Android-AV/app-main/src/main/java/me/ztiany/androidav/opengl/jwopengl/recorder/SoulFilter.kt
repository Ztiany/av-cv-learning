package me.ztiany.androidav.opengl.jwopengl.recorder

import android.opengl.GLES20
import me.ztiany.androidav.opengl.jwopengl.gles2.*

/**灵魂出鞘效果，注意：其接收来自相机的纹理。*/
class SoulFilter : BaseGLFilter() {

    private var glFBO: GLFBOWithTexture? = null

    /**矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /**纹理坐标*/
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateAndroid())

    private var mixPercent = 0F
    private var scalePercent = 0F

    private var textureWidth = 0
    private var textureHeight = 0

    override fun createAndInitProgram(): GLProgram {
        val glProgram = GLProgram.fromAssets(
            "shader/vertex_base.glsl",
            "shader/fragment_soul_OES.glsl"
        )

        //vertex
        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")

        //fragment
        glProgram.activeUniform("scalePercent")
        glProgram.activeUniform("mixPercent")
        glProgram.activeUniform("uTexture")

        return glProgram
    }

    override fun setWorldSize(width: Int, height: Int) {

    }

    override fun setTextureAttribute(attribute: TextureAttribute) {
        this.textureWidth = attribute.width
        this.textureHeight = attribute.height
    }

    override fun onDrawFrame(sharedTexture: GLTexture): GLTexture {
        val fbo = getFBO()

        fbo.use {
            GLES20.glViewport(0, 0, textureWidth, textureHeight)
            doDraw(sharedTexture)
        }

        return fbo.texture
    }

    private fun getFBO(): GLFBOWithTexture {
        var fbo = glFBO

        if (fbo != null && (fbo.texture.width != textureWidth || fbo.texture.height != textureHeight)) {
            fbo.delete()
            fbo = null
        }

        if (fbo == null) {
            val glTexture = generateFBOTexture(
                glProgram.uniformHandle("uTexture"),
                0,
                //use the real texture size.
                textureWidth,
                textureHeight
            )
            fbo = generateFBOWithTexture(glTexture)
            glFBO = fbo
        }

        return fbo
    }

    private fun doDraw(sharedTexture: GLTexture) {
        glProgram.startDraw {
            clearColorBuffer()
            //vertex
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            //fragment
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