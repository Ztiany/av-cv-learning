package me.ztiany.av.webrtc

import android.util.Log
import org.webrtc.SdpObserver
import org.webrtc.SessionDescription

open class SimpleSdpObserver(private val tag: String) : SdpObserver {

    override fun onCreateSuccess(sessionDescription: SessionDescription) {
        Log.i(tag, "SdpObserver: onCreateSuccess !")
    }

    override fun onSetSuccess() {
        Log.i(tag, "SdpObserver: onSetSuccess")
    }

    override fun onCreateFailure(msg: String) {
        Log.e(tag, "SdpObserver onCreateFailure: $msg")
    }

    override fun onSetFailure(msg: String) {
        Log.e(tag, "SdpObserver onSetFailure: $msg")
    }

}