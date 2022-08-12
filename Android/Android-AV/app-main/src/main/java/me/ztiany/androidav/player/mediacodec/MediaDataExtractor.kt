package me.ztiany.androidav.player.mediacodec

import android.media.MediaFormat
import android.net.Uri
import me.ztiany.lib.avbase.utils.MediaMetadata
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
    fun prepare(): MediaInfo

    fun readAudioPacket(buffer: ByteBuffer, packet: PacketInfo? = null): Int

    fun readVideoPacket(buffer: ByteBuffer, packet: PacketInfo? = null): Int

    /**
     * Seek 到指定位置，并返回实际帧的时间戳
     */
    fun seek(position: Long)

    fun release()

}

class MediaInfo(
    val metadata: MediaMetadata,
    val audioFormat: MediaFormat?,
    val videoFormat: MediaFormat?
)

typealias TrackSelector = (MediaFormat) -> Boolean

val AUDIO_SELECTOR: TrackSelector = {
    it.getString(MediaFormat.KEY_MIME)?.startsWith("audio/") ?: false
}

val VIDEO_SELECTOR: TrackSelector = {
    it.getString(MediaFormat.KEY_MIME)?.startsWith("video/") ?: false
}

data class PacketInfo(
    var sampleTime: Long,
    var sampleFlags: Int
)

