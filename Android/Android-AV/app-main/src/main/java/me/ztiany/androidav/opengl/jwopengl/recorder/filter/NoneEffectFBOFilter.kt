package me.ztiany.androidav.opengl.jwopengl.recorder.filter

import android.opengl.GLES20
import me.ztiany.androidav.opengl.jwopengl.gles2.*
import timber.log.Timber

class NoneEffectFBOFilter : BaseGLFilter() {

    private var glFBO: GLFBOWithTexture? = null

    /**矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /**纹理坐标*/
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateAndroid())

    private var textureWidth = 0
    private var textureHeight = 0

    override fun createAndInitProgram(): GLProgram {
        Timber.d("createAndInitProgram() called")

        val glProgram = GLProgram.fromAssets(
            "shader/vertex_base.glsl",
            "shader/fragment_camera.glsl"
        )

        //vertex
        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")

        //fragment
        glProgram.activeUniform("uTexture")

        return glProgram
    }

    override fun setWorldSize(width: Int, height: Int) {

    }

    override fun setTextureAttribute(attribute: TextureAttribute) {
        this.textureWidth = attribute.width
        this.textureHeight = attribute.height
    }

    override fun doDraw(sharedTexture: GLTexture): GLTexture {
        val fbo = getFBO()
        fbo.use {
            glProgram.startDraw {
                clearColorBuffer()
                GLES20.glViewport(0, 0, textureWidth, textureHeight)
                //vertex
                vertexAttribPointerFloat("aPosition", 3, vertexVbo)
                vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
                //texture【将分享过来的视频纹理绘制到 FBO 的纹理上】
                sharedTexture.activeTexture(uniformHandle("uTexture"))
                //draw
                drawArraysStrip(4/*4 个顶点*/)
            }
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
            Timber.d("create new fbo $textureWidth x $textureHeight.")
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

}