package me.ztiany.androidav.opengl.jwopengl.recorder

import android.content.Context
import android.graphics.SurfaceTexture
import android.opengl.EGL14
import android.opengl.EGLContext
import android.opengl.GLES11Ext
import androidx.core.content.ContextCompat
import me.ztiany.androidav.opengl.jwopengl.common.EGLBridger
import me.ztiany.androidav.opengl.jwopengl.common.GLRenderer
import me.ztiany.androidav.opengl.jwopengl.gles2.GLTexture
import me.ztiany.androidav.opengl.jwopengl.gles2.TextureAttribute
import me.ztiany.androidav.opengl.jwopengl.gles2.generateTexture
import timber.log.Timber

/**录像特效 + 展示*/
class RecorderShowRenderer(
    private val context: Context,
    private val eglBridger: EGLBridger
) : GLRenderer {

    /**承载视频的纹理*/
    private lateinit var cameraSurfaceTexture: SurfaceTexture
    private lateinit var cameraTexture: GLTexture
    private var onSurfaceText: ((SurfaceTexture) -> Unit)? = null

    private val soulFilter = SoulFilter()
    private val screenFilter = ScreenFilter()

    @Volatile private var recorder: Recorder? = null

    private lateinit var eglContext: EGLContext

    fun startRecording(recorder: Recorder) {
        this.recorder?.onStop()
        this.recorder = recorder
        this.recorder?.onStart(eglContext)
    }

    fun stopRecording() {
        val stoppingRecorder = this.recorder
        this.recorder = null
        stoppingRecorder?.onStop()
    }

    fun getSurfaceTexture(onSurfaceText: (SurfaceTexture) -> Unit) {
        if (::cameraSurfaceTexture.isInitialized) {
            onSurfaceText(cameraSurfaceTexture)
        } else {
            this.onSurfaceText = onSurfaceText
        }
    }

    override fun onSurfaceCreated() {
        eglContext = EGL14.eglGetCurrentContext()

        cameraTexture = generateTexture(
            0,
            0,
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES
        )

        cameraSurfaceTexture = SurfaceTexture(cameraTexture.name)

        ContextCompat.getMainExecutor(context).execute {
            onSurfaceText?.invoke(cameraSurfaceTexture)
            onSurfaceText = null
        }

        cameraSurfaceTexture.setOnFrameAvailableListener {
            onFrameAvailable(it)
        }

        soulFilter.initProgram()
        screenFilter.initProgram()
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        Timber.d("onSurfaceChanged")
        soulFilter.setWorldSize(width, height)
        screenFilter.setWorldSize(width, height)
    }

    override fun onDrawFrame(attachment: Any?) {
        cameraSurfaceTexture.updateTexImage()
        var glTexture = soulFilter.onDrawFrame(cameraTexture)
        glTexture = screenFilter.onDrawFrame(glTexture)

        recorder?.run {
            onFrame(TextureWithTime(glTexture, cameraSurfaceTexture.timestamp))
        }
    }

    override fun onSurfaceDestroy() {
        Timber.d("onSurfaceDestroy")
    }

    private fun onFrameAvailable(surfaceTexture: SurfaceTexture) {
        eglBridger.requestRender()
    }

    fun setVideoAttribute(attribute: TextureAttribute) {
        Timber.d("setVideoAttribute() called with: attribute = $attribute")
        soulFilter.setTextureAttribute(attribute)
        screenFilter.setTextureAttribute(attribute)
    }

}