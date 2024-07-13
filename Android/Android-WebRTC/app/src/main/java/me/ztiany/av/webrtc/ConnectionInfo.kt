package me.ztiany.av.webrtc

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class ConnectionInfo(
    val signalServer: String,
    val roomName: String,
    val iceServerList: List<IceServerInfo> = emptyList()
) : Parcelable

@Parcelize
data class IceServerInfo(
    val url: String,
    val username: String,
    val password: String
) : Parcelable {

    fun toIceServer(): org.webrtc.PeerConnection.IceServer {
        return org.webrtc.PeerConnection.IceServer.builder(url)
            .setUsername(username)
            .setPassword(password)
            .createIceServer()
    }

}