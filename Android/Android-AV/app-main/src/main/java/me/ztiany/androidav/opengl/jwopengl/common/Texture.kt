package me.ztiany.androidav.opengl.jwopengl.common

import android.graphics.Bitmap
import android.opengl.GLES20
import android.opengl.GLUtils
import android.util.Log
import timber.log.Timber

data class GLTexture(
    val textureId: Int,
    val width: Int,
    val height: Int,
    val index: Int
)

fun generateTexture(bitmap: Bitmap, textureIndex: Int): GLTexture {
    //创建纹理对象
    val textureObjectIds = IntArray(1)
    GLES20.glGenTextures(1, textureObjectIds, 0)
    if (textureObjectIds[0] == 0) {
        Timber.w("Could not generate a new OpenGL texture object.")
        throw IllegalStateException()
    }

    GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureObjectIds[0])

    //设置纹理过滤参数:解决纹理缩放过程中的锯齿问题。若不设置，则会导致纹理为黑色
    GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR_MIPMAP_LINEAR)
    GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR)
    GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT)
    GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT)

    //通过OpenGL对象读取Bitmap数据，并且绑定到纹理对象上，之后就可以回收Bitmap对象
    GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0)

    //解绑
    GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0)

    return GLTexture(textureObjectIds[0], bitmap.width, bitmap.height, textureIndex)
}

fun mapTextureIndex(textureIndex: Int): Int {
    return when (textureIndex) {
        0 -> GLES20.GL_TEXTURE0
        1 -> GLES20.GL_TEXTURE1
        2 -> GLES20.GL_TEXTURE2
        3 -> GLES20.GL_TEXTURE3
        4 -> GLES20.GL_TEXTURE4
        5 -> GLES20.GL_TEXTURE5
        6 -> GLES20.GL_TEXTURE6
        7 -> GLES20.GL_TEXTURE7
        8 -> GLES20.GL_TEXTURE8
        9 -> GLES20.GL_TEXTURE9
        else -> throw IllegalArgumentException()
    }
}