package me.ztiany.rtmp.encoder

import me.ztiany.rtmp.source.VideoConfig

interface VideoEncoder {

    fun initEncoder(videoConfig: VideoConfig): Boolean

}