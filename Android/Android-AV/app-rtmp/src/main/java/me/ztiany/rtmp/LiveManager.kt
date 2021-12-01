package me.ztiany.rtmp

import me.ztiany.rtmp.encoder.VideoEncoder
import me.ztiany.rtmp.source.VideoConfig
import me.ztiany.rtmp.source.VideoSource

class LiveManager(
    private val videoConfig: VideoConfig,
    private val videoSource: VideoSource,
    private val videoEncoder: VideoEncoder
) {

    fun start() {
        videoEncoder.initEncoder(videoConfig)
        videoSource.start(videoConfig)
    }

}