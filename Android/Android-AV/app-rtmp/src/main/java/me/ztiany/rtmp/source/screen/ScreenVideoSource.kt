package me.ztiany.rtmp.source.screen

import android.hardware.display.DisplayManager
import android.hardware.display.VirtualDisplay
import android.media.projection.MediaProjection
import me.ztiany.rtmp.source.VideoConfig
import me.ztiany.rtmp.source.VideoSource

class ScreenVideoSource(
    private val mediaProjection: MediaProjection,
) : VideoSource {

    private var virtualDisplay: VirtualDisplay? = null

    override fun start(videoConfig: VideoConfig) {
        val surface = videoConfig.surface ?: throw NullPointerException()

        virtualDisplay = mediaProjection.createVirtualDisplay(
            "screen-codec",
            videoConfig.targetWidth,
            videoConfig.targetHeight,
            1,
            DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC,
            surface,
            null,
            null
        );
    }

    override fun stop() {
        virtualDisplay?.release()
    }

}