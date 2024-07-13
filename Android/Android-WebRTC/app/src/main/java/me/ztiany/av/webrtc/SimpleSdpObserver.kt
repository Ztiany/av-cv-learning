package me.ztiany.av.webrtc

import android.util.Log
import org.webrtc.SdpObserver
import org.webrtc.SessionDescription

class SimpleSdpObserver : SdpObserver {
    override fun onCreateSuccess(sessionDescription: SessionDescription) {
        Log.i(TAG, "SdpObserver: onCreateSuccess !")
    }

    override fun onSetSuccess() {
        Log.i(TAG, "SdpObserver: onSetSuccess")
    }

    override fun onCreateFailure(msg: String) {
        Log.e(TAG, "SdpObserver onCreateFailure: $msg")
    }

    override fun onSetFailure(msg: String) {
        Log.e(TAG, "SdpObserver onSetFailure: $msg")
    }
}