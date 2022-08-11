package me.ztiany.androidav.player.mediacodec

interface MediaDataProvider {

    fun getPacket(): Packet?

}