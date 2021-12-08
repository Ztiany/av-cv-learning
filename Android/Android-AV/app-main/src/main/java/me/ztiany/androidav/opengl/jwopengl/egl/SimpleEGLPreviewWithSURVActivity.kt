package me.ztiany.androidav.opengl.jwopengl.egl

import android.graphics.Point
import android.util.Size
import me.ztiany.androidav.databinding.OpenglActivityEglSimpleSurvPreviewBinding
import me.ztiany.androidav.opengl.jwopengl.egl.core.EGLAttribute
import me.ztiany.androidav.opengl.jwopengl.egl.core.EGLEnvironment
import me.ztiany.androidav.opengl.jwopengl.egl.core.SurfaceViewProvider
import me.ztiany.androidav.opengl.jwopengl.egl.renderer.CameraRenderer
import me.ztiany.androidav.opengl.oglcamera.CameraBuilder
import me.ztiany.androidav.opengl.oglcamera.CameraListener
import me.ztiany.androidav.opengl.oglcamera.CameraOperator
import me.ztiany.lib.avbase.BaseActivity

class SimpleEGLPreviewWithSURVActivity : BaseActivity<OpenglActivityEglSimpleSurvPreviewBinding>() {

    private lateinit var eglEnvironment: EGLEnvironment
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
        eglEnvironment = EGLEnvironment(SurfaceViewProvider(binding.openglSurfaceView), EGLAttribute())
        cameraRenderer = CameraRenderer(this, eglEnvironment)
        eglEnvironment.renderer = cameraRenderer
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
                    binding.openglSurfaceView.width,
                    binding.openglSurfaceView.height
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
        eglEnvironment.release()
    }

}