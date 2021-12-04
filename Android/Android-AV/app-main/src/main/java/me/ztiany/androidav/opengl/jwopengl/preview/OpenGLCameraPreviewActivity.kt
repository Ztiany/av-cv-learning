package me.ztiany.androidav.opengl.jwopengl.preview

import android.graphics.Point
import android.opengl.GLSurfaceView
import android.util.Size
import me.ztiany.androidav.databinding.OpenglActivityJavaCameraPreviewBinding
import me.ztiany.androidav.opengl.oglcamera.*
import me.ztiany.lib.avbase.BaseActivity

class OpenGLCameraPreviewActivity : BaseActivity<OpenglActivityJavaCameraPreviewBinding>() {

    private lateinit var cameraOperator: CameraOperator
    private lateinit var cameraRenderer: CameraRenderer

    private fun onCameraAvailable(previewSize: Size, displayOrientation: Int) {
        if ((displayOrientation / 90).mod(2) == 1) {
            cameraRenderer.setVideoAttribute(previewSize.height, previewSize.width, displayOrientation)
        } else {
            cameraRenderer.setVideoAttribute(previewSize.width, previewSize.height, displayOrientation)
        }
        cameraRenderer.getSurfaceTexture {
            cameraOperator.startPreview(it)
        }
    }

    override fun setUpView() {
        setUpGlSurfaceView()
        setUpCamera()
    }

    private fun setUpGlSurfaceView() {
        binding.openglCameraView.setEGLContextClientVersion(2)
        cameraRenderer = CameraRenderer(this, binding.openglCameraView)
        binding.openglCameraView.setRenderer(cameraRenderer)
        binding.openglCameraView.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
    }

    private fun setUpCamera() {
        val cameraListener = CameraListener { _, _, previewSize, displayOrientation, _ ->
            onCameraAvailable(previewSize, displayOrientation)
        }

        cameraOperator = CameraBuilder()
            .cameraListener(cameraListener)
            .maxPreviewSize(Point(1920, 1080))
            .minPreviewSize(Point(100, 100))
            .specificCameraId(CameraOperator.CAMERA_ID_BACK)
            .context(applicationContext)
            .previewViewSize(
                Point(
                    binding.openglCameraView.width,
                    binding.openglCameraView.height
                )
            )
            .targetPreviewSize(Point(800, 480))
            .rotation(windowManager.defaultDisplay.rotation)
            .build("2")
    }

    override fun onResume() {
        super.onResume()
        cameraOperator.start()
    }

    override fun onPause() {
        super.onPause()
        cameraOperator.stop()
    }

    override fun onDestroy() {
        super.onDestroy()
        cameraOperator.release()
    }

}