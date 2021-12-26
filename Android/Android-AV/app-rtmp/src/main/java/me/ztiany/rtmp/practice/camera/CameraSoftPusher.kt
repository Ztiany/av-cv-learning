package me.ztiany.rtmp.practice.camera

import android.hardware.camera2.CameraDevice
import android.util.Size
import android.view.TextureView
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.lib.avbase.utils.YUVUtils
import me.ztiany.rtmp.common.Pusher
import me.ztiany.rtmp.common.RtmpPusher
import me.ztiany.rtmp.common.RtmpPusher.VIDEO_TYPE_YUV

class CameraSoftPusher(
    private val cameraId: String,
    context: AppCompatActivity,
    textureView: TextureView
) : Pusher {

    private val camera2VideoSource = Camera2VideoSource(context, textureView)

    private val rtmpPusher = RtmpPusher.getInstance()
    private var nv21: ByteArray? = null

    private var rtmpInitSucceeded = false

    private val rtmpCallback = object : RtmpPusher.Callback {
        override fun onInitFailed() = Unit
        override fun onInitSuccess() {
            rtmpInitSucceeded = true
        }
    }

    private val camera2Listener = object : Camera2Listener {

        override fun onCameraOpened(
            cameraDevice: CameraDevice,
            cameraId: String,
            previewSize: Size,
            displayOrientation: Int,
            isMirror: Boolean
        ) {
            if ((displayOrientation / 90).mod(2) == 1) {//竖屏
                rtmpPusher.initVideoCodec(previewSize.height, previewSize.width, 15, 800_000, RtmpPusher.VIDEO_FORMAT_I420)
            } else {//横屏
                rtmpPusher.initVideoCodec(previewSize.width, previewSize.height, 15, 800_000, RtmpPusher.VIDEO_FORMAT_I420)
            }
        }

        override fun onPreview(y: ByteArray, u: ByteArray, v: ByteArray, previewSize: Size, stride: Int) {
            var buffer = nv21
            if (buffer == null || buffer.size != previewSize.width * previewSize.height * 3 / 2) {
                buffer = ByteArray(previewSize.width * previewSize.height * 3 / 2)
            }
            if (rtmpInitSucceeded) {
                YUVUtils.i420FromYUVCutToWidth(y, u, v, buffer, stride, previewSize.width, previewSize.height)
                rtmpPusher.sendVideoPacket(buffer, VIDEO_TYPE_YUV, 0L)
            }
        }

        override fun onCameraClosed() {

        }

        override fun onCameraError(e: Exception?) {
        }

    }

    override fun start(url: String) {
        camera2VideoSource.start(480, 800, cameraId, camera2Listener)
        rtmpPusher.init()
        rtmpPusher.setCallback(rtmpCallback)
        rtmpPusher.start(url)
    }

    override fun stop() {
        camera2VideoSource.stop()
        rtmpPusher.stop()
        rtmpPusher.release()
        rtmpPusher.setCallback(null)
    }

}