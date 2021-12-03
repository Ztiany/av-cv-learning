package me.ztiany.androidav.opengl.jwopengl.common

import android.opengl.GLES20
import me.ztiany.androidav.common.FileUtils
import java.nio.FloatBuffer

class GLProgram(
    vertexSource: String,
    fragmentSource: String
) {

    private val programHandle = generateGLProgram(vertexSource, fragmentSource)

    private val attributeMap = mutableMapOf<String, Int>()
    private val uniformMap = mutableMapOf<String, Int>()

    fun setSize(width: Int, height: Int) {

    }

    companion object {
        fun fromAssets(vertexPath: String, fragmentPath: String) = GLProgram(FileUtils.loadAssets(vertexPath), FileUtils.loadAssets(fragmentPath))
    }

    fun activeAttribute(attributeName: String) {
        val attribLocation = GLES20.glGetAttribLocation(programHandle, attributeName)
        attributeMap[attributeName] = attribLocation
    }

    fun activeUniform(uniformName: String) {
        val attribLocation = GLES20.glGetUniformLocation(programHandle, uniformName)
        uniformMap[uniformName] = attribLocation
    }

    fun setColor(red: Float, green: Float, blue: Float, alpha: Float) {
        GLES20.glClearColor(red, green, blue, alpha)
    }

    fun clearColorBuffer() {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)
    }

    fun clearBuffer(bufferMask: Int) {
        GLES20.glClear(bufferMask)
    }

    fun startDraw(onDraw: GLProgram.() -> Unit) {
        GLES20.glUseProgram(programHandle)

        attributeMap.values.forEach {
            GLES20.glEnableVertexAttribArray(it)
        }

        onDraw(this)

        attributeMap.values.forEach {
            GLES20.glDisableVertexAttribArray(it)
        }
        GLES20.glUseProgram(0)
    }

    /**
     * - elementsPerVertex: 每个点几个 float。
     */
    fun vertexAttribPointerFloat(attribute: String, elementsPerVertex: Int, vbo: FloatBuffer) {
        GLES20.glVertexAttribPointer(
            attributeHandle(attribute),
            elementsPerVertex,
            GLES20.GL_FLOAT,
            false,
            elementsPerVertex * 4/*每个点 4 个 float，每 float 4 byte*/,
            vbo
        )
    }

    /**
     * - elementsPerVertex: 每个点几个 float。
     */
    fun vertexAttribPointerInt(attribute: String, elementsPerVertex: Int, vbo: FloatBuffer) {
        GLES20.glVertexAttribPointer(
            attributeHandle(attribute),
            elementsPerVertex,
            GLES20.GL_FLOAT,
            false,
            elementsPerVertex * 4/*每个点 4 个 float，每 float 4 byte*/,
            vbo
        )
    }

    private fun attributeHandle(attribute: String) = attributeMap[attribute] ?: throw NoSuchElementException()

    private fun uniformHandle(attribute: String) = uniformMap[attribute] ?: throw NoSuchElementException()

}