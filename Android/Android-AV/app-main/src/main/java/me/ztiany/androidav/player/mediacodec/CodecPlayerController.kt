package me.ztiany.androidav.player.mediacodec

import android.content.Context
import android.media.MediaFormat
import android.net.Uri
import android.view.Surface
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import me.ztiany.lib.avbase.utils.MediaMetadata
import timber.log.Timber
import java.nio.ByteBuffer

class CodecPlayerController(
    private val context: Context
) {

    private val stateHolder = CodecPlayerStateHolder()

    private val mediaDataExtractor: MediaDataExtractor by lazy {
        MediaDataExtractorImpl(context)
    }

    private var audioDecoder: MediaDataDecoder? = null
    private var audioRenderer: MediaDataRenderer? = null

    private var videoDecoder: MediaDataDecoder? = null
    private var videoRenderer: MediaDataRenderer? = null

    var mediaInfo: MediaInfo? = null
        private set

    var listener: ControllerListener? = null

    fun setDataSource(source: Uri) {
        mediaDataExtractor.setSource(source)
    }

    fun setVideoRenderer(surface: Surface) {
        if (stateHolder.isStarted) {
            throw UnsupportedOperationException("Set a surface before you start.")
        }
    }

    fun setVideoRenderer(videoRenderer: MediaDataRenderer) {
        if (stateHolder.isStarted) {
            throw UnsupportedOperationException("Set a MediaDataRenderer before you start.")
        }
        this.videoRenderer = videoRenderer
    }

    suspend fun prepare(): Boolean {
        if (!stateHolder.switchToPrepared()) {
            return false
        }

        return withContext(Dispatchers.IO) {
            try {
                val mediaInfo = mediaDataExtractor.prepare()
                checkMediaInfoAndInitDecoder(mediaInfo.metadata, mediaInfo.audioFormat, mediaInfo.videoFormat)
            } catch (e: Exception) {
                Timber.e(e, "prepare error")
                stateHolder.switchToNone()
                false
            }
        }
    }

    fun start() {
        if (stateHolder.switchToStarted()) {
            Timber.d("started")
            audioDecoder?.start()
            videoDecoder?.start()
        } else {
            Timber.d("start failed")
        }
    }

    private fun initAudioDecoder(mediaMetadata: MediaMetadata, audioFormat: MediaFormat?) {
        if (audioFormat == null) {
            Timber.w("initAudioDecoder no audioFormat provide.")
            return
        }
        audioDecoder = AudioDataDecoder(
            audioFormat,
            stateHolder,
            object : MediaDataProvider {
                override fun readPacket(buffer: ByteBuffer, packet: PacketInfo?): Int {
                    return mediaDataExtractor.readAudioPacket(buffer, packet)
                }
            },
            PCMAudioDataRenderer().apply {
                updateMediaFormat(audioFormat)
                audioRenderer = this
            }
        )
    }

    private fun initVideoDecoder(mediaMetadata: MediaMetadata, videoFormat: MediaFormat?) {

    }

    fun pause() {
        stateHolder.switchToPause()
    }

    fun stop() {
        stateHolder.switchToStop()
        mediaDataExtractor.release()
    }

    fun release() {
        stop()
        audioDecoder?.stop()
        videoDecoder?.stop()
        audioRenderer?.release()
        videoRenderer?.release()
        mediaDataExtractor.release()
    }

    val isPlaying: Boolean
        get() = stateHolder.isStarted

    private fun checkMediaInfoAndInitDecoder(metaData: MediaMetadata, audioFormat: MediaFormat?, videoFormat: MediaFormat?): Boolean {
        Timber.d("checkMediaInfo() called with: metaData = $metaData, audioFormat = $audioFormat, videoFormat = $videoFormat")

        if (audioFormat == null && videoFormat == null) {
            listener?.onError(ERROR_UNSUPPORTED)
            return false
        }

        var metaInfo = metaData
        if (videoFormat != null) {
            val videoWidth = videoFormat.getInteger(MediaFormat.KEY_WIDTH)
            val videoHeight = videoFormat.getInteger(MediaFormat.KEY_HEIGHT)
            val duration = videoFormat.getLong(MediaFormat.KEY_DURATION)
            metaInfo = metaData.copy(width = videoWidth, height = videoHeight, duration = duration.toInt())
            Timber.d("checkMediaInfo update video info %s", metaInfo.toString())
        }

        initAudioDecoder(metaData, audioFormat)
        initVideoDecoder(metaData, videoFormat)

        mediaInfo = MediaInfo(metaInfo, audioFormat, videoFormat)

        return true
    }

    interface ControllerListener {

        fun onError(code: Int)

    }

    companion object {
        const val ERROR_UNSUPPORTED = 1
    }

}