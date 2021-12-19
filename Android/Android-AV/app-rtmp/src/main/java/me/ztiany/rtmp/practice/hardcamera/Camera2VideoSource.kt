package me.ztiany.rtmp.practice.hardcamera

import android.graphics.Point
import android.hardware.camera2.CameraDevice
import android.util.Size
import android.view.TextureView
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.rtmp.common.Camera2Helper
import me.ztiany.rtmp.common.Camera2Listener
import me.ztiany.rtmp.practice.screen.VideoConfig

class Camera2VideoSource(
    private val context: AppCompatActivity,
    private val textureView: TextureView
) {

    private var camera2Helper: Camera2Helper? = null

    private val camera2Listener = object : Camera2Listener {

        override fun onCameraOpened(
            cameraDevice: CameraDevice,
            cameraId: String,
            previewSize: Size,
            displayOrientation: Int,
            isMirror: Boolean
        ) {

        }

        override fun onPreview(y: ByteArray, u: ByteArray, v: ByteArray, previewSize: Size, stride: Int) {

        }

        override fun onCameraClosed() {
        }

        override fun onCameraError(e: Exception?) {
        }

    }

    fun start(videoConfig: VideoConfig) {
        initCamera(videoConfig)
    }

    private fun initCamera(videoConfig: VideoConfig) {
        Camera2Helper.Builder()
            .cameraListener(camera2Listener)
            .maxPreviewSize(Point(1920, 1080))
            .minPreviewSize(Point(1280, 720))
            .previewViewSize(Point(textureView.width, textureView.height))
            .previewSize(Point(videoConfig.targetHeight, videoConfig.targetWidth))
            .specificCameraId(Camera2Helper.CAMERA_ID_BACK)
            .context(context.application)
            .previewOn(textureView)
            .rotation(context.windowManager.defaultDisplay.rotation)
            .build().apply {
                camera2Helper = this
                start()
            }
    }

    fun stop() {
        camera2Helper?.stop()
    }

}