package me.ztiany.androidav.player.mediacodec

import android.content.Context
import android.media.MediaFormat
import android.net.Uri
import android.view.Surface

class CodecPlayerController(
    private val context: Context
) {

    private val bufferPool = ByteBufferPool()
    private val stateHolder = CodecPlayerStateHolder()

    private val mediaDataExtractor: MediaDataExtractor by lazy {
        MediaDataExtractorImpl(context, bufferPool)
    }

    private var audioDecoder: MediaDataDecoder? = null
    private var audioRenderer: MediaDataRenderer? = null

    private var videoDecoder: MediaDataDecoder? = null
    private var videoRenderer: MediaDataRenderer? = null

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

    fun start() {
        if (stateHolder.switchToStarted()) {
            mediaDataExtractor.invokeOnPrepared { audioFormat, videoFormat ->
                initAudioDecoder(audioFormat)
                initVideoDecoder(videoFormat)
                audioDecoder?.start()
                videoDecoder?.start()
            }
            mediaDataExtractor.start()
        }
    }

    private fun initAudioDecoder(audioFormat: MediaFormat?) {
        if (audioFormat == null) {
            return
        }
        audioDecoder = AudioDataDecoder(
            audioFormat,
            stateHolder,
            object : MediaDataProvider {
                override fun getPacket(): Packet? {
                    return mediaDataExtractor.getAudioPacket()
                }
            },
            PCMAudioDataRenderer()
        )
    }

    private fun initVideoDecoder(videoFormat: MediaFormat?) {

    }

    fun pause() {

    }

    fun resume() {

    }

    fun stop() {

    }

    fun release() {

    }

}