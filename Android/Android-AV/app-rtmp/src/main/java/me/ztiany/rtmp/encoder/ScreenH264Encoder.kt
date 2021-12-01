package me.ztiany.rtmp.encoder

import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.media.MediaFormat
import android.view.Surface
import me.ztiany.rtmp.source.VideoConfig

class ScreenH264Encoder : VideoEncoder {

    private var mediaCodec: MediaCodec? = null

    override fun initEncoder(videoConfig: VideoConfig): Boolean {
        val format = MediaFormat.createVideoFormat(
            MediaFormat.MIMETYPE_VIDEO_AVC,
            videoConfig.targetWidth,
            videoConfig.targetHeight
        )

        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface)
        format.setInteger(MediaFormat.KEY_BIT_RATE, 400000)
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 15)
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1)

        try {
            mediaCodec = MediaCodec.createEncoderByType("video/avc").apply {
                configure(
                    format, null, null,
                    MediaCodec.CONFIGURE_FLAG_ENCODE
                )
                val surface: Surface = createInputSurface()
                videoConfig.surface = surface
            }
            return true
        } catch (e: Exception) {
            return false
        }

    }


}