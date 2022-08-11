package me.ztiany.androidav.player.mediacodec

import android.media.MediaFormat
import android.net.Uri
import java.io.IOException
import java.nio.ByteBuffer

interface MediaDataExtractor {

    fun setSource(source: Uri)

    fun setAudioTrackSelector(selector: TrackSelector)

    fun setVideoTrackSelector(selector: TrackSelector)

    /**
     * @throws IllegalStateException 没有提前调用 [setSource], [setAudioTrackSelector], [setVideoTrackSelector] 方法。
     * @throws IOException 数据源错误
     */
    fun start()

    fun invokeOnPrepared(onPrepared: (audioFormat: MediaFormat?, videoFormat: MediaFormat?) -> Unit)

    fun getAudioPacket(): Packet?

    fun getVideoPacket(): Packet?

    /**
     * Seek 到指定位置，并返回实际帧的时间戳
     */
    fun seek(position: Long)

    fun stop()

}

typealias TrackSelector = (MediaFormat) -> Boolean

val AUDIO_SELECTOR: TrackSelector = {
    it.getString(MediaFormat.KEY_MIME)?.startsWith("audio/") ?: false
}

val VIDEO_SELECTOR: TrackSelector = {
    it.getString(MediaFormat.KEY_MIME)?.startsWith("video/") ?: false
}

data class Packet(
    val data: ByteBuffer,
    val size: Int,
    val sampleTime: Long,
    val sampleFlags: Int
)

