package me.ztiany.androidav.player.mediacodec

import java.nio.ByteBuffer

class ByteBufferPool {

    fun getByteBuffer(size: Long): ByteBuffer {
        return ByteBuffer.allocate(size.toInt())
    }

    fun releaseByteBuffer(size: Long): ByteBuffer {
        return ByteBuffer.allocate(size.toInt())
    }

}