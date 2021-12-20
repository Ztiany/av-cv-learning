package me.ztiany.rtmp.common

import android.graphics.Point
import android.view.TextureView
import androidx.appcompat.app.AppCompatActivity

class Camera2VideoSource(
    private val context: AppCompatActivity,
    private val textureView: TextureView
) {

    private var camera2Helper: Camera2Helper? = null

    fun start(targetWidth: Int, targetHeight: Int, cameraId: String, camera2Listener: Camera2Listener) {
        Camera2Helper.Builder()
            .cameraListener(camera2Listener)
            .maxPreviewSize(Point(1920, 1080))
            .minPreviewSize(Point(1280, 720))
            .previewViewSize(Point(textureView.width, textureView.height))
            .previewSize(Point(targetHeight, targetWidth))/*摄像头的取景方向不同*/
            .specificCameraId(cameraId)
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