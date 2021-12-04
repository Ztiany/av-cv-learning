package me.ztiany.androidav.opengl.jwopengl.common

import android.graphics.Bitmap
import android.opengl.GLES20
import android.opengl.GLES11Ext
import android.opengl.GLUtils
import timber.log.Timber

class GLTexture(
    val handle: Int,
    val id: Int,
    val width: Int,
    val height: Int,
    val index: Int,
    val textureType: Int
)

fun GLTexture.activeTexture() {
    //激活指定纹理单元【有 32 个纹理单位，默认只有 0 号纹理单元是激活的】为了代码的统一性，不管哪个纹理，都调用一次 glActiveTexture，多次调用没有问题。
    GLES20.glActiveTexture(getTextureIdentificationByIndex(index))
    //绑定纹理 ID 到当前激活的纹理单元。
    GLES20.glBindTexture(textureType, id)
    //将激活的纹理单元传递到着色器里面。
    //我们使用 glUniform1i 设置 uniform 采样器的位置值，或者说纹理单元。通过 glUniform1i 的设置，我们保证每个 uniform 采样器对应着正确的纹理单元。
    GLES20.glUniform1i(handle, index)
}

/**
 * - [textureHandle]：纹理句柄
 * - [textureType]: 比如 [GLES20.GL_TEXTURE_2D]，或者 [GLES11Ext.GL_TEXTURE_EXTERNAL_OES] 等。
 * - [textureIndex]：0-31 闭区间
 */
fun generateTexture(
    textureHandle: Int,
    textureIndex: Int,
    textureType: Int = GLES20.GL_TEXTURE_2D
): GLTexture {
    //申请纹理
    val textureObjectIds = allocateTexture()
    //设置通用属性
    setCommonAttribute(textureType, textureObjectIds)
    //返回纹理封装对象
    return GLTexture(textureHandle, textureObjectIds[0], 0, 0, textureIndex, textureType)
}

/**
 * - [textureHandle]：纹理句柄
 * - [bitmap]：纹理图片
 * - [textureIndex]：0-31 闭区间
 */
fun generateTextureFromBitmap(
    textureHandle: Int,
    textureIndex: Int,
    bitmap: Bitmap
): GLTexture {
    //申请纹理
    val textureObjectIds = allocateTexture()

    //设置通用属性
    val textureType = GLES20.GL_TEXTURE_2D
    setCommonAttribute(textureType, textureObjectIds)

    //通过 OpenGL 对象读取 Bitmap 数据，并且绑定到纹理对象上，绑定之后就可以回收 Bitmap 对象。
    GLES20.glBindTexture(textureType, textureObjectIds[0])
    GLUtils.texImage2D(textureType, 0, bitmap, 0)
    GLES20.glBindTexture(textureType, 0)

    //返回纹理封装对象
    return GLTexture(textureHandle, textureObjectIds[0], bitmap.width, bitmap.height, textureIndex, textureType)
}

private fun setCommonAttribute(textureType: Int, textureObjectIds: IntArray) {
    //绑定到这个纹理，之后的纹理操作就是对这个纹理进行操作
    GLES20.glBindTexture(textureType, textureObjectIds[0])

    //设置纹理缩放过滤
    // GL_NEAREST: 使用纹理中坐标最接近的一个像素的颜色作为需要绘制的像素颜色
    // GL_LINEAR: 使用纹理中坐标最接近的若干个颜色，通过加权平均算法得到需要绘制的像素颜色，速度较慢，但视觉效果好
    GLES20.glTexParameteri(textureType, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR)
    GLES20.glTexParameteri(textureType, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR)

    //纹理坐标的范围是 0-1。超出这一范围的坐标将被 OpenGL 根据 GL_TEXTURE_WRAP 参数的值进行处理
    //  GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T 分别为 x，y 方向。
    //  GL_REPEAT: 平铺。
    //  GL_MIRRORED_REPEAT: 纹理坐标是奇数时使用镜像平铺。
    //  GL_CLAMP_TO_EDGE:: 坐标超出部分被截取成 0、1，边缘拉伸。
    GLES20.glTexParameteri(textureType, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT)
    GLES20.glTexParameteri(textureType, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT)

    //操作完了，就解除绑定
    GLES20.glBindTexture(textureType, 0)
}

private fun allocateTexture(): IntArray {
    //创建纹理对象
    val textureObjectIds = IntArray(1)
    GLES20.glGenTextures(1, textureObjectIds, 0)
    if (textureObjectIds[0] == 0) {
        Timber.w("Could not generate a new OpenGL texture object.")
        throw IllegalStateException()
    }

    return textureObjectIds
}

/**OpenGL 一共有 32 个纹理，从 0-31 号纹理都有对应的标识，定义在 [GLES20] 中。*/
private fun getTextureIdentificationByIndex(textureIndex: Int): Int {
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
        10 -> GLES20.GL_TEXTURE10
        11 -> GLES20.GL_TEXTURE11
        12 -> GLES20.GL_TEXTURE12
        13 -> GLES20.GL_TEXTURE13
        14 -> GLES20.GL_TEXTURE14
        15 -> GLES20.GL_TEXTURE15
        16 -> GLES20.GL_TEXTURE16
        17 -> GLES20.GL_TEXTURE17
        18 -> GLES20.GL_TEXTURE18
        19 -> GLES20.GL_TEXTURE19
        20 -> GLES20.GL_TEXTURE20
        21 -> GLES20.GL_TEXTURE21
        22 -> GLES20.GL_TEXTURE22
        23 -> GLES20.GL_TEXTURE23
        24 -> GLES20.GL_TEXTURE24
        25 -> GLES20.GL_TEXTURE25
        26 -> GLES20.GL_TEXTURE26
        27 -> GLES20.GL_TEXTURE27
        28 -> GLES20.GL_TEXTURE28
        29 -> GLES20.GL_TEXTURE29
        30 -> GLES20.GL_TEXTURE30
        31 -> GLES20.GL_TEXTURE31
        else -> throw IllegalArgumentException()
    }
}