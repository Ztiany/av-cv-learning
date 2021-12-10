package me.ztiany.androidav.opengl.jwopengl

import android.graphics.Point
import android.opengl.GLSurfaceView
import android.util.Size
import me.ztiany.androidav.databinding.OpenglActivityRecorderBinding
import me.ztiany.androidav.opengl.jwopengl.common.CompoundRenderer
import me.ztiany.androidav.opengl.jwopengl.common.EGLBridger
import me.ztiany.androidav.opengl.jwopengl.painter.RecorderShowPainter
import me.ztiany.androidav.opengl.oglcamera.CameraBuilder
import me.ztiany.androidav.opengl.oglcamera.CameraListener
import me.ztiany.androidav.opengl.oglcamera.CameraOperator
import me.ztiany.lib.avbase.BaseActivity

class OpenGLRecorderActivity : BaseActivity<OpenglActivityRecorderBinding>() {

    private lateinit var cameraOperator: CameraOperator
    private lateinit var recorderShowPainter: RecorderShowPainter

    private fun onCameraAvailable(previewSize: Size, displayOrientation: Int, isMirror: Boolean) {
        if ((displayOrientation / 90).mod(2) == 1) {
            recorderShowPainter.setVideoAttribute(previewSize.height, previewSize.width, displayOrientation)
        } else {
            recorderShowPainter.setVideoAttribute(previewSize.width, previewSize.height, displayOrientation)
        }
        recorderShowPainter.getSurfaceTexture {
            cameraOperator.startPreview(it)
        }
    }

    override fun setUpView() {
        setUpGlSurfaceView()
        setUpCamera()
    }

    private fun setUpGlSurfaceView() {
        binding.openglCameraView.setEGLContextClientVersion(2)
        recorderShowPainter = RecorderShowPainter(this, object : EGLBridger {
            override fun requestRender() {
                binding.openglCameraView.requestRender()
            }
        })
        binding.openglCameraView.setRenderer(CompoundRenderer(recorderShowPainter))
        binding.openglCameraView.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
    }

    private fun setUpCamera() {
        val cameraListener = CameraListener { _, cameraID, previewSize, displayOrientation ->
            onCameraAvailable(previewSize, displayOrientation, CameraOperator.CAMERA_ID_FRONT == cameraID)
        }

        val openglCameraView = binding.openglCameraView

        cameraOperator = CameraBuilder()
            .cameraListener(cameraListener)
            .maxPreviewSize(Point(1920, 1080))
            .minPreviewSize(Point(100, 100))
            .specificCameraId(CameraOperator.CAMERA_ID_BACK)
            .context(applicationContext)
            .previewViewSize(Point(openglCameraView.width, openglCameraView.height))
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