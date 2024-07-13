package me.ztiany.av.webrtc

import android.annotation.SuppressLint
import android.content.Intent
import android.os.Bundle
import android.os.Looper
import android.util.Log
import android.view.View
import androidx.activity.OnBackPressedCallback
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.IntentCompat
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import me.ztiany.av.webrtc.databinding.ActivityRoomBinding
import org.json.JSONException
import org.json.JSONObject
import org.webrtc.IceCandidate

private const val TAG = "RoomActivity"

class RoomActivity : AppCompatActivity() {

    private val vb by lazy { ActivityRoomBinding.inflate(layoutInflater) }

    private val connectionInfo by lazy {
        IntentCompat.getParcelableExtra(intent, CONNECTION_INFO, ConnectionInfo::class.java)
            ?: throw IllegalArgumentException("ConnectionInfo is required.")
    }

    private val signalClient: SignalClient by lazy {
        SignalClient(
            connectionInfo.signalServer,
            connectionInfo.roomName,
            signalEventListener
        ) { appendLog(it) }
    }

    private val webRtcClient: WebRtcClient by lazy {
        WebRtcClient(
            vb = vb,
            context = this,
            signalClient = signalClient,
            iceServerInfos = connectionInfo.iceServerList
        ) { appendLog(it) }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(vb.root)
        enableEdgeToEdge()

        signalClient.joinRoom()

        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                confirmToExit(true)
            }
        })
    }

    override fun onResume() {
        super.onResume()
        webRtcClient.resume()
    }

    override fun onPause() {
        super.onPause()
        webRtcClient.pause()
    }

    override fun onDestroy() {
        super.onDestroy()
        signalClient.leaveRoom()
        webRtcClient.release()
    }

    private val signalEventListener = object : SignalClient.SignalEventListener {

        override fun onConnected() = Unit

        override fun onConnectingFailed() {
            confirmToExit()
        }

        override fun onDisconnected() {
            confirmToExit()
        }

        override fun onUserJoined(roomName: String, userId: String) {
            webRtcClient.initPeerConnection()
        }

        override fun onUserLeft(roomName: String, userId: String) {
            confirmToExit()
        }

        override fun onRemoteUserJoined(roomName: String) {
            webRtcClient.initPeerConnection()
            webRtcClient.sendOffer()
        }

        override fun onRemoteUserLeft(roomName: String, userId: String) {
            webRtcClient.releasePeerConnection()
            lifecycleScope.launch { vb.surfaceRemote.visibility = View.GONE }
        }

        override fun onRoomFull(roomName: String, userId: String) {
            confirmToExit()
        }

        override fun onMessage(message: JSONObject) {
            try {
                when (val type = message.getString("type")) {
                    "offer" -> onRemoteOfferReceived(message)
                    "answer" -> onRemoteAnswerReceived(message)
                    "candidate" -> onRemoteCandidateReceived(message)
                    else -> Log.e(TAG, "Unknown message type: $type")
                }
            } catch (e: JSONException) {
                e.printStackTrace()
            }
        }

        private fun onRemoteOfferReceived(message: JSONObject) {
            appendLog("onRemoteOfferReceived: $message")
            webRtcClient.initPeerConnection()
            val description = message.getString("sdp")
            webRtcClient.setRemoteOffer(description)
            webRtcClient.sendAnswer()
            lifecycleScope.launch { vb.surfaceRemote.visibility = View.VISIBLE }
        }

        private fun onRemoteAnswerReceived(message: JSONObject) {
            appendLog("onRemoteAnswerReceived: $message")
            lifecycleScope.launch { vb.surfaceRemote.visibility = View.VISIBLE }
            val description = message.getString("sdp")
            webRtcClient.setRemoteAnswer(description)
        }

        private fun onRemoteCandidateReceived(message: JSONObject) {
            appendLog("onRemoteCandidateReceived: $message")
            val remoteIceCandidate = IceCandidate(
                message.getString("id"),
                message.getInt("label"),
                message.getString("candidate")
            )
            webRtcClient.addIceCandidate(remoteIceCandidate)
        }

    }

    @SuppressLint("SetTextI18n")
    private fun appendLog(msg: String) {
        lifecycleScope.launch {
            vb.tvLog.text = "${vb.tvLog.getText()}\n\n${msg}"
            delay(100)
            vb.svLog.scrollTo(0, vb.tvLog.height)
        }
    }

    private fun confirmToExit(byUser: Boolean = false) {
        if (isDestroyed || isFinishing) {
            return
        }

        if (Looper.myLooper() != Looper.getMainLooper()) {
            lifecycleScope.launch { confirmToExit(byUser) }
            return
        }

        AlertDialog.Builder(this)
            .setTitle("Exit")
            .setMessage(
                if (byUser) {
                    "Are you sure to exit?"
                } else {
                    "The connection is lost, please exit!"
                }
            )
            .setPositiveButton("Exit") { _, _ ->
                supportFinishAfterTransition()
            }
            .apply {
                if (byUser) {
                    setNegativeButton("Cancel") { _, _ -> finish() }
                }
            }
            .setCancelable(false)
            .show()
    }

    companion object {
        private const val CONNECTION_INFO = "connection_info"

        fun start(activity: AppCompatActivity, connectionInfo: ConnectionInfo) {
            activity.startActivity(
                Intent(activity, RoomActivity::class.java).apply { putExtra(CONNECTION_INFO, connectionInfo) }
            )
        }
    }

}