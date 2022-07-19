package me.ztiany.androidav.player.mediacodec

import android.media.MediaFormat
import androidx.annotation.WorkerThread
import java.nio.ByteBuffer

interface MediaDataProvider {

    @WorkerThread
    fun read(buffer: ByteBuffer): Int

    fun getCurrentBufferInfo(): BufferInfo

    fun getMediaFormat(): MediaFormat

}