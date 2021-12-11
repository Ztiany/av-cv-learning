package me.ztiany.androidav.opengl.jwopengl.recorder

import me.ztiany.androidav.opengl.jwopengl.gles2.GLProgram
import me.ztiany.androidav.opengl.jwopengl.gles2.GLTexture
import me.ztiany.androidav.opengl.jwopengl.gles2.TextureAttribute

interface GLFilter {

    fun initProgram()

    fun setWorldSize(width: Int, height: Int)

    fun setTextureAttribute(attribute: TextureAttribute)

    fun onDrawFrame(sharedTexture: GLTexture): GLTexture

}

abstract class BaseGLFilter : GLFilter {

    private lateinit var _glProgram: GLProgram

    protected val glProgram: GLProgram
        get() = _glProgram

    override fun initProgram() {
        _glProgram = createAndInitProgram()
    }

    protected abstract fun createAndInitProgram(): GLProgram

    override fun setWorldSize(width: Int, height: Int) {
    }

    override fun setTextureAttribute(attribute: TextureAttribute) {
    }

}