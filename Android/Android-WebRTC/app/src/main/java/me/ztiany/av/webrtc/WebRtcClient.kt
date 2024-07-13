package me.ztiany.av.webrtc

import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.av.webrtc.databinding.ActivityRoomBinding
import org.json.JSONException
import org.json.JSONObject
import org.webrtc.AudioTrack
import org.webrtc.Camera1Enumerator
import org.webrtc.Camera2Enumerator
import org.webrtc.CameraEnumerator
import org.webrtc.DataChannel
import org.webrtc.DefaultVideoDecoderFactory
import org.webrtc.DefaultVideoEncoderFactory
import org.webrtc.EglBase
import org.webrtc.IceCandidate
import org.webrtc.Logging
import org.webrtc.MediaConstraints
import org.webrtc.MediaStream
import org.webrtc.PeerConnection
import org.webrtc.PeerConnection.IceConnectionState
import org.webrtc.PeerConnection.IceGatheringState
import org.webrtc.PeerConnection.RTCConfiguration
import org.webrtc.PeerConnection.SignalingState
import org.webrtc.PeerConnectionFactory
import org.webrtc.RendererCommon
import org.webrtc.RtpReceiver
import org.webrtc.SessionDescription
import org.webrtc.SurfaceTextureHelper
import org.webrtc.VideoCapturer
import org.webrtc.VideoDecoderFactory
import org.webrtc.VideoEncoderFactory
import org.webrtc.VideoTrack


private const val VIDEO_TRACK_ID: String = "ARDAMSv0"
private const val AUDIO_TRACK_ID: String = "ARDAMSa0"

private const val VIDEO_RESOLUTION_WIDTH: Int = 1280
private const val VIDEO_RESOLUTION_HEIGHT: Int = 720
private const val VIDEO_FPS: Int = 30

private const val TAG: String = "WebRtcClient"

class WebRtcClient(
    private val vb: ActivityRoomBinding,
    private val context: AppCompatActivity,
    private val signalClient: SignalClient,
    private val iceServerInfos: List<IceServerInfo> = emptyList(),
    private val onNewLog: (String) -> Unit
) {

    private val peerConnectionFactory: PeerConnectionFactory

    private var peerConnection: PeerConnection? = null

    private val rootEglBase: EglBase = EglBase.create()

    private val surfaceTextureHelper: SurfaceTextureHelper

    private val videoTrack: VideoTrack
    private val audioTrack: AudioTrack

    private val videoCapturer: VideoCapturer

    ///////////////////////////////////////////////////////////////////////////
    // initialization
    ///////////////////////////////////////////////////////////////////////////

    init {
        with(vb.surfaceLocal) {
            init(rootEglBase.eglBaseContext, null)
            setScalingType(RendererCommon.ScalingType.SCALE_ASPECT_FILL)
            setMirror(true)
            setEnableHardwareScaler(true)
        }

        with(vb.surfaceRemote) {
            init(rootEglBase.eglBaseContext, null)
            setScalingType(RendererCommon.ScalingType.SCALE_ASPECT_FILL)
            setMirror(true)
            setEnableHardwareScaler(true)
            setZOrderMediaOverlay(true)
        }

        peerConnectionFactory = createPeerConnectionFactory()

        // NOTE: this _must_ happen while PeerConnectionFactory is alive!
        Logging.enableLogToDebugOutput(Logging.Severity.LS_NONE);

        videoCapturer = createVideoCapturer()

        surfaceTextureHelper = SurfaceTextureHelper.create("CaptureThread", rootEglBase.eglBaseContext)

        val videoSource = peerConnectionFactory.createVideoSource(false)
        videoCapturer.initialize(
            surfaceTextureHelper,
            context.applicationContext,
            videoSource.capturerObserver
        )

        videoTrack = peerConnectionFactory.createVideoTrack(VIDEO_TRACK_ID, videoSource).apply {
            setEnabled(true)
            addSink(vb.surfaceLocal)
        }

        val audioSource = peerConnectionFactory.createAudioSource(MediaConstraints())
        audioTrack = peerConnectionFactory.createAudioTrack(AUDIO_TRACK_ID, audioSource).apply {
            setEnabled(true)
        }

        onNewLog("Webrtc components initialized!")
    }

    private fun createPeerConnectionFactory(): PeerConnectionFactory {
        val encoderFactory: VideoEncoderFactory = DefaultVideoEncoderFactory(
            rootEglBase.eglBaseContext,
            false,
            true
        )

        val decoderFactory: VideoDecoderFactory = DefaultVideoDecoderFactory(rootEglBase.eglBaseContext)

        PeerConnectionFactory.initialize(
            PeerConnectionFactory.InitializationOptions.builder(context).setEnableInternalTracer(true).createInitializationOptions()
        )

        return PeerConnectionFactory.builder()
            .setVideoEncoderFactory(encoderFactory)
            .setVideoDecoderFactory(decoderFactory)
            .setOptions(null)
            .createPeerConnectionFactory()
    }

    private fun createVideoCapturer(): VideoCapturer {
        return if (Camera2Enumerator.isSupported(context)) {
            createCameraCapturer(Camera2Enumerator(context));
        } else {
            createCameraCapturer(Camera1Enumerator(true));
        }
    }

    private fun createCameraCapturer(enumerator: CameraEnumerator): VideoCapturer {
        val deviceNames = enumerator.deviceNames

        // First, try to find front facing camera.
        Log.d(TAG, "Looking for front facing cameras.")
        for (deviceName in deviceNames) {
            if (enumerator.isFrontFacing(deviceName)) {
                Log.d(TAG, "Creating front facing camera capturer.")
                val videoCapturer: VideoCapturer? = enumerator.createCapturer(deviceName, null)
                if (videoCapturer != null) {
                    return videoCapturer
                }
            }
        }

        // Front facing camera not found, try something else
        Log.d(TAG, "Looking for other cameras.")
        for (deviceName in deviceNames) {
            if (!enumerator.isFrontFacing(deviceName)) {
                Log.d(TAG, "Creating other camera capturer.")
                val videoCapturer: VideoCapturer? = enumerator.createCapturer(deviceName, null)
                if (videoCapturer != null) {
                    return videoCapturer
                }
            }
        }

        throw RuntimeException("Failed to open capturer")
    }

    fun initPeerConnection() {
        if (peerConnection == null) {
            peerConnection = createPeerConnection()
        }
    }

    fun releasePeerConnection() {
        onNewLog("Release PeerConnection")
        peerConnection?.close()
        peerConnection = null
    }

    private fun createPeerConnection(): PeerConnection? {
        Log.i(TAG, "Create PeerConnection")

        val rtcConfig = RTCConfiguration(iceServerInfos.map { it.toIceServer() }).apply {
            // TCP candidates are only useful when connecting to a server that supports ICE-TCP.
            tcpCandidatePolicy = PeerConnection.TcpCandidatePolicy.DISABLED

            //bundlePolicy = PeerConnection.BundlePolicy.MAXBUNDLE;
            //rtcpMuxPolicy = PeerConnection.RtcpMuxPolicy.REQUIRE;

            continualGatheringPolicy = PeerConnection.ContinualGatheringPolicy.GATHER_CONTINUALLY

            // Use ECDSA encryption.
            //keyType = PeerConnection.KeyType.ECDSA;

            // Enable DTLS for normal calls and disable for loopback calls.
            enableDtlsSrtp = true

            //sdpSemantics = PeerConnection.SdpSemantics.UNIFIED_PLAN;
        }

        peerConnection = peerConnectionFactory.createPeerConnection(rtcConfig, peerConnectionObserver)

        if (peerConnection == null) {
            Log.e(TAG, "Failed to createPeerConnection!")
            onNewLog("Failed to createPeerConnection!")
            return null
        }

        Log.d(TAG, "Create PeerConnection success!")
        onNewLog("Create PeerConnection success!")

        peerConnection?.run {
            val mediaStreamLabels = listOf("ARDAMS")
            addTrack(videoTrack, mediaStreamLabels)
            addTrack(audioTrack, mediaStreamLabels)
        }

        return peerConnection
    }

    private val peerConnectionObserver: PeerConnection.Observer = object : PeerConnection.Observer {

        override fun onSignalingChange(signalingState: SignalingState) {
            Log.i(TAG, "onSignalingChange: $signalingState")
        }

        override fun onIceConnectionChange(iceConnectionState: IceConnectionState) {
            Log.i(TAG, "onIceConnectionChange: $iceConnectionState")
        }

        override fun onIceConnectionReceivingChange(b: Boolean) {
            Log.i(TAG, "onIceConnectionChange: $b")
        }

        override fun onIceGatheringChange(iceGatheringState: IceGatheringState) {
            Log.i(TAG, "onIceGatheringChange: $iceGatheringState")
        }

        override fun onIceCandidate(iceCandidate: IceCandidate) {
            Log.i(TAG, "onIceCandidate: $iceCandidate")
            try {
                val message = JSONObject()
                message.put("type", "candidate")
                message.put("label", iceCandidate.sdpMLineIndex)
                message.put("id", iceCandidate.sdpMid)
                message.put("candidate", iceCandidate.sdp)
                signalClient.sendMessage(message)
            } catch (e: JSONException) {
                e.printStackTrace()
            }
        }

        override fun onIceCandidatesRemoved(iceCandidates: Array<IceCandidate>) {
            for (i in iceCandidates.indices) {
                Log.i(TAG, "onIceCandidatesRemoved: " + iceCandidates[i])
            }
            peerConnection?.removeIceCandidates(iceCandidates)
        }

        override fun onAddStream(mediaStream: MediaStream) {
            Log.i(TAG, "onAddStream: " + mediaStream.videoTracks.size)
        }

        override fun onRemoveStream(mediaStream: MediaStream) {
            Log.i(TAG, "onRemoveStream")
        }

        override fun onDataChannel(dataChannel: DataChannel) {
            Log.i(TAG, "onDataChannel")
        }

        override fun onRenegotiationNeeded() {
            Log.i(TAG, "onRenegotiationNeeded")
        }

        override fun onAddTrack(rtpReceiver: RtpReceiver, mediaStreams: Array<MediaStream>) {
            val track = rtpReceiver.track()
            if (track is VideoTrack) {
                Log.i(TAG, "onAddVideoTrack")
                track.setEnabled(true)
                track.addSink(vb.surfaceRemote)
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // actions
    ///////////////////////////////////////////////////////////////////////////

    fun resume() {
        videoCapturer.startCapture(VIDEO_RESOLUTION_WIDTH, VIDEO_RESOLUTION_HEIGHT, VIDEO_FPS)
    }

    fun pause() {
        try {
            videoCapturer.stopCapture()
        } catch (e: InterruptedException) {
            Log.e(TAG, "stopCapture error: $e")
        }
    }

    fun release() {
        // do not change the order of the following code.
        releasePeerConnection()
        vb.surfaceLocal.release()
        vb.surfaceRemote.release()
        videoCapturer.dispose()
        surfaceTextureHelper.dispose()
        PeerConnectionFactory.stopInternalTracingCapture()
        PeerConnectionFactory.shutdownInternalTracer()
        peerConnectionFactory.dispose()
    }

    fun sendOffer() {
        val connection = peerConnection ?: return

        val mediaConstraints = MediaConstraints().apply {
            mandatory.add(MediaConstraints.KeyValuePair("OfferToReceiveAudio", "true"))
            mandatory.add(MediaConstraints.KeyValuePair("OfferToReceiveVideo", "true"))
            optional.add(MediaConstraints.KeyValuePair("DtlsSrtpKeyAgreement", "true"))
        }

        connection.createOffer(object : SimpleSdpObserver(TAG) {
            override fun onCreateSuccess(sessionDescription: SessionDescription) {
                Log.i(TAG, "create local offer success: ${sessionDescription.description}")
                connection.setLocalDescription(SimpleSdpObserver(TAG), sessionDescription)
                val message = JSONObject()
                try {
                    message.put("type", "offer")
                    message.put("sdp", sessionDescription.description)
                    signalClient.sendMessage(message)
                } catch (e: JSONException) {
                    Log.e(TAG, "sendOffer error: $e")
                    onNewLog("sendOffer error: $e")
                }
            }
        }, mediaConstraints)
    }

    fun setRemoteOffer(description: String) {
        val connection = peerConnection ?: return
        try {
            connection.setRemoteDescription(SimpleSdpObserver(TAG), SessionDescription(SessionDescription.Type.OFFER, description))
        } catch (e: JSONException) {
            Log.e(TAG, "setRemoteOffer error: $e")
            onNewLog("setRemoteOffer error: $e")
        }
    }

    fun sendAnswer() {
        val connection = peerConnection ?: return
        val sdpMediaConstraints = MediaConstraints()
        Log.i(TAG, "Create answer ...")
        connection.createAnswer(object : SimpleSdpObserver(TAG) {
            override fun onCreateSuccess(sessionDescription: SessionDescription) {
                Log.i(TAG, "create local answer success: ${sessionDescription.description}")
                connection.setLocalDescription(SimpleSdpObserver(TAG), sessionDescription)

                val message = JSONObject()
                try {
                    message.put("type", "answer")
                    message.put("sdp", sessionDescription.description)
                    signalClient.sendMessage(message)
                } catch (e: JSONException) {
                    Log.e(TAG, "sendAnswer error: $e")
                    onNewLog("sendAnswer error: $e")
                }
            }
        }, sdpMediaConstraints)
    }

    fun setRemoteAnswer(description: String) {
        try {
            peerConnection?.setRemoteDescription(SimpleSdpObserver(TAG), SessionDescription(SessionDescription.Type.ANSWER, description))
        } catch (e: JSONException) {
            Log.e(TAG, "setRemoteDescription error: $e")
            onNewLog("setRemoteDescription error: $e")
        }
    }

    fun addIceCandidate(remoteIceCandidate: IceCandidate) {
        if (peerConnection == null) {
            Log.w(TAG, "addIceCandidate: peerConnection is null!")
            onNewLog("addIceCandidate: peerConnection is null!")
        }
        try {
            peerConnection?.addIceCandidate(remoteIceCandidate)
        } catch (e: JSONException) {
            Log.e(TAG, "addIceCandidate error: $e")
            onNewLog("addIceCandidate error: $e")
        }
    }

}