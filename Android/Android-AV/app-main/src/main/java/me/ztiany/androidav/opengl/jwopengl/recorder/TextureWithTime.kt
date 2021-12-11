package me.ztiany.androidav.opengl.jwopengl.recorder

import me.ztiany.androidav.opengl.jwopengl.gles2.GLTexture

class TextureWithTime(
    /**存储了当前帧的纹理*/
    val glTexture: GLTexture,
    /**单位：nanoseconds*/
    val timestamp: Long
)
