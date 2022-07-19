package me.ztiany.androidav.player.mediacodec

interface MediaDataDecoder {

    fun setMediaDataProvider(provider: MediaDataProvider)

    fun setMediaDataRenderer(renderer: MediaDataRenderer)

    fun initMediaDecoder()

    fun start()

    fun stop()

    fun release()

}