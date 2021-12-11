package me.ztiany.androidav.opengl.jwopengl.recorder

import android.graphics.SurfaceTexture
import android.opengl.EGLContext
import android.opengl.GLSurfaceView
import android.util.Size
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.opengl.jwopengl.common.EGLBridger
import me.ztiany.androidav.opengl.jwopengl.common.setGLRenderer
import me.ztiany.androidav.opengl.jwopengl.gles2.TextureAttribute

class RecorderManager(
    private val context: AppCompatActivity
) {

    private var showRenderer: RecorderShowRenderer? = null
    private var encodeRenderer: RecorderEncodeRenderer? = null

    @Volatile var isRecording = false
        private set

    @Volatile private var isOperating = false

    var speed = Speed.MODE_NORMAL

    fun init(openglCameraView: GLSurfaceView) {
        //decode
        encodeRenderer = RecorderEncodeRenderer()

        //shower
        openglCameraView.setEGLContextClientVersion(2)
        showRenderer = RecorderShowRenderer(openglCameraView.context, object : EGLBridger {
            override fun requestRender() {
                openglCameraView.requestRender()
            }
        }).also {
            openglCameraView.setGLRenderer(it)
            openglCameraView.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        }
    }

    fun onCameraAvailable(previewSize: Size, displayOrientation: Int, isFront: Boolean, startPreview: (SurfaceTexture) -> Unit) {
        val textureAttribute = TextureAttribute(previewSize.width, previewSize.height, displayOrientation, isFront, false)
        showRenderer?.setVideoAttribute(textureAttribute)
        encodeRenderer?.setVideoAttribute(textureAttribute)
        showRenderer?.getSurfaceTexture {
            startPreview(it)
        }
    }

    fun startRecording(path: String) {
        if (isRecording || isOperating) {
            return
        }
        isRecording = true
        isOperating = true

        showRenderer?.startRecording(object : Recorder {
            override fun onStart(sharedEglContext: EGLContext) {
                encodeRenderer?.start(sharedEglContext, newEncoder(path))
                isOperating = false
            }

            override fun onFrame(frame: TextureWithTime) {
                encodeRenderer?.onFrame(frame)
            }

            override fun onStop() {
                encodeRenderer?.stop()
                isOperating = false
            }
        })
    }

    fun stopRecording() {
        if (!isRecording || isOperating) {
            return
        }
        isRecording = false
        isOperating = true
        showRenderer?.stopRecording()
    }

    private fun newEncoder(path: String): Encoder {
        return HardEncoder(path, speed.getSpeedValue())
    }

}