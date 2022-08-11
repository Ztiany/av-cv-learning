package me.ztiany.androidav.player.mediacodec

import android.media.MediaCodec
import android.media.MediaCodecList
import android.media.MediaFormat
import timber.log.Timber
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.concurrent.thread

class AudioDataDecoder(
    private val mediaFormat: MediaFormat,
    private val stateHolder: CodecPlayerStateHolder,
    private val provider: MediaDataProvider,
    private val renderer: MediaDataRenderer
) : MediaDataDecoder {

    private lateinit var decoder: MediaCodec
    private val isRunning = AtomicBoolean(false)
    private var reachEndOfStream = false

    private val outputBufferInfo by lazy {
        MediaCodec.BufferInfo()
    }

    private fun initAudioDecoder(mediaFormat: MediaFormat) {
        val name = MediaCodecList(MediaCodecList.ALL_CODECS).findDecoderForFormat(mediaFormat)
        Timber.d("initAudioDecoder.findDecoderForFormat: $name")
        decoder = MediaCodec.createByCodecName(name)
        decoder.configure(mediaFormat, null, null, 0)
    }

    override fun start() {
        if (isRunning.compareAndSet(false, true)) {
            thread {
                startDecoding()
            }
        }
    }

    private fun startDecoding() {
        decoder.start()
        while (isRunning.get()) {
            pushData()
            pullData()
        }
    }

    private fun pushData() {
        /*if (reachEndOfStream) {
            Timber.d("AudioDataDecoder pushData reachEndOfStream")
            return
        }

        val inputBufferIndex = decoder.dequeueInputBuffer(-1)
        if (inputBufferIndex < 0) {
            Timber.d("AudioDataDecoder pushData inputBufferIndex < 0")
            return
        }
        val byteBuffer = decoder.getInputBuffer(inputBufferIndex) ?: return
        val sampleSize = provider.read(byteBuffer)
        val bufferInfo = provider.getCurrentBufferInfo()
        Timber.d("AudioDataDecoder pushData bufferInfo = $bufferInfo")
        if (sampleSize < 0) {
            decoder.queueInputBuffer(inputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM)
            reachEndOfStream = true
        } else {
            decoder.queueInputBuffer(inputBufferIndex, 0, sampleSize, bufferInfo.sampleTime, 0)
        }*/
    }

    private fun pullData() {
        val index = decoder.dequeueOutputBuffer(outputBufferInfo, 1000)
        when {
            index == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED -> {

            }
            index == MediaCodec.INFO_TRY_AGAIN_LATER -> {

            }
            index == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED -> {

            }
            index == MediaCodec.BUFFER_FLAG_END_OF_STREAM -> {
                stop()
            }
            index >= 0 -> {
                decoder.getOutputBuffer(index)?.let {
                    renderer.render(it, outputBufferInfo)
                    decoder.releaseOutputBuffer(index, false)
                }
            }
        }
    }

    private fun release() {
        decoder.stop()
        decoder.release()
    }

    override fun stop() {
        isRunning.set(false)
    }

}