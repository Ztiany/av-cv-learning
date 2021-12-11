package me.ztiany.androidav.opengl.jwopengl.gles2

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

data class TextureAttribute(
    val width: Int,
    val height: Int,
    val orientation: Int = 0,
    val isFront: Boolean = false,
    val isCamera: Boolean = false
)