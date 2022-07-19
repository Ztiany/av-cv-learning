package me.ztiany.androidav.player.mediacodec

import android.media.*
import me.ztiany.lib.avbase.utils.getChannelOutConfig
import timber.log.Timber
import java.nio.ByteBuffer

class PCMAudioDataRenderer : MediaDataRenderer {

    private var audioTrack: AudioTrack? = null

    private lateinit var audioOutTempBuffer: ShortArray

    override fun initRenderer(mediaFormat: MediaFormat) {
        val channelCount = mediaFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT)
        val sampleRate = mediaFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE)
        val pcmEncoding = if (mediaFormat.containsKey(MediaFormat.KEY_PCM_ENCODING)) {
            mediaFormat.getInteger(MediaFormat.KEY_PCM_ENCODING)
        } else {
            AudioFormat.ENCODING_PCM_16BIT
        }

        val channelConfig = getChannelOutConfig(channelCount)
        val minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelConfig, pcmEncoding)

        audioTrack = AudioTrack(
            AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                .build(),
            AudioFormat.Builder()
                .setSampleRate(sampleRate)
                .setEncoding(pcmEncoding)
                .setChannelMask(channelConfig)
                .build(),
            minBufferSize,
            AudioTrack.MODE_STREAM,
            AudioManager.AUDIO_SESSION_ID_GENERATE
        )

        audioTrack?.play()
    }

    override fun render(mediaData: ByteBuffer, bufferInfo: MediaCodec.BufferInfo) {
        audioTrack?.let {
            if (!::audioOutTempBuffer.isInitialized || audioOutTempBuffer.size < bufferInfo.size / 2) {
                audioOutTempBuffer = ShortArray(bufferInfo.size / 2)
            }
            mediaData.position(0)
            mediaData.asShortBuffer().get(audioOutTempBuffer, 0, bufferInfo.size / 2)
            val result = it.write(audioOutTempBuffer, 0, bufferInfo.size / 2)
            if (result < 0) {
                Timber.e("playAudioInStreamMode: ${getErrorMessage(result)}")
            }
        }
    }

    private fun getErrorMessage(result: Int): String {
        return when (result) {
            AudioTrack.ERROR_INVALID_OPERATION -> "play fail: ERROR_INVALID_OPERATION"
            AudioTrack.ERROR_BAD_VALUE -> "play fail: ERROR_BAD_VALUE"
            AudioManager.ERROR_DEAD_OBJECT -> "play fail: ERROR_DEAD_OBJECT"
            else -> "play fail: null mAudioTrack"
        }
    }

    override fun releaseRenderer() {
        val track = audioTrack
        audioTrack = null
        track?.run {
            stop()
            release()
        }
    }

}