package me.ztiany.androidav.opengl.jwopengl

import android.graphics.Point
import android.opengl.GLSurfaceView
import android.util.Size
import me.ztiany.androidav.databinding.OpenglActivityRecorderBinding
import me.ztiany.androidav.opengl.jwopengl.common.EGLBridger
import me.ztiany.androidav.opengl.jwopengl.common.setGLRenderer
import me.ztiany.androidav.opengl.jwopengl.gles2.TextureAttribute
import me.ztiany.androidav.opengl.jwopengl.recorder.RecorderEncodeRenderer
import me.ztiany.androidav.opengl.jwopengl.recorder.RecorderShowRenderer
import me.ztiany.androidav.opengl.oglcamera.CameraBuilder
import me.ztiany.androidav.opengl.oglcamera.CameraListener
import me.ztiany.androidav.opengl.oglcamera.CameraOperator
import me.ztiany.lib.avbase.BaseActivity

class OpenGLRecorderActivity : BaseActivity<OpenglActivityRecorderBinding>() {

    private lateinit var cameraOperator: CameraOperator
    private lateinit var showRenderer: RecorderShowRenderer
    private lateinit var encodeRenderer: RecorderEncodeRenderer
    private var isRecording = false

    private fun onCameraAvailable(previewSize: Size, displayOrientation: Int, isFront: Boolean) {
        val textureAttribute = TextureAttribute(previewSize.width, previewSize.height, displayOrientation, isFront, false)
        showRenderer.setVideoAttribute(textureAttribute)
        encodeRenderer.setVideoAttribute(textureAttribute)
        showRenderer.getSurfaceTexture {
            cameraOperator.startPreview(it)
        }
    }

    override fun setUpView() {
        setUpGlSurfaceView()
        setUpEGL()
        setUpCamera()

        binding.openglBtStart.setOnClickListener {
            if (!isRecording) {
                showRenderer.getEGLContext { eglContext ->
                    encodeRenderer.start(eglContext)
                    showRenderer.onFrame = { glTexture, timestamp ->
                        encodeRenderer.onFrame(glTexture, timestamp)
                    }
                }
                isRecording = true
            } else {
                encodeRenderer.stop()
                isRecording = false
            }
        }
    }

    private fun setUpEGL() {
        encodeRenderer = RecorderEncodeRenderer(this)
    }

    private fun setUpGlSurfaceView() {
        binding.openglCameraView.setEGLContextClientVersion(2)
        showRenderer = RecorderShowRenderer(this, object : EGLBridger {
            override fun requestRender() {
                binding.openglCameraView.requestRender()
            }
        })
        binding.openglCameraView.setGLRenderer(showRenderer)
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
            .specificCameraId(CameraOperator.CAMERA_ID_FRONT)
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