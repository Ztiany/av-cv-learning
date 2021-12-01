package me.ztiany.rtmp.source

interface VideoSource {

    fun start(videoConfig: VideoConfig)

    fun stop()

}