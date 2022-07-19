package me.ztiany.androidav.player.mediacodec

import android.media.MediaFormat
import android.net.Uri
import androidx.annotation.WorkerThread
import java.io.IOException
import java.nio.ByteBuffer

interface MediaDataExtractor : MediaDataProvider {

    fun setSource(source: Uri)

    fun setTrackSelector(selector: TrackSelector)

    /**
     * @throws IllegalStateException 没有提前调用 [setSource] 或 [setTrackSelector] 方法
     * @throws IOException 数据源错误
     */
    @WorkerThread
    fun initExtractor()

    @WorkerThread
    override fun read(buffer: ByteBuffer): Int

    fun release()

    override fun getCurrentBufferInfo(): BufferInfo

    /**
     * Seek 到指定位置，并返回实际帧的时间戳
     */
    fun seek(position: Long): Long

    override fun getMediaFormat(): MediaFormat

}

typealias TrackSelector = (MediaFormat) -> Boolean

val AUDIO_SELECTOR: TrackSelector = {
    it.getString(MediaFormat.KEY_MIME)?.startsWith("audio/") ?: false
}

val VIDEO_SELECTOR: TrackSelector = {
    it.getString(MediaFormat.KEY_MIME)?.startsWith("video/") ?: false
}

data class BufferInfo(
    var sampleTime: Long,
    var sampleFlags: Int,
    var track: Int
)