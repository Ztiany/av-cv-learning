package me.ztiany.androidav.opengl.jwopengl.painter

import me.ztiany.androidav.opengl.jwopengl.common.GLProgram
import me.ztiany.androidav.opengl.jwopengl.common.GLTexture

interface GLFilter {

    fun initProgram()

    fun setWorldSize(width: Int, height: Int)

    fun setTextureAttribute(width: Int, height: Int, orientation: Int)

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

    override fun setTextureAttribute(width: Int, height: Int, orientation: Int) {
    }

}