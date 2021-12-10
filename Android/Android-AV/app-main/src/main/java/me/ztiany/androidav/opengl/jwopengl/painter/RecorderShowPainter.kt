package me.ztiany.androidav.opengl.jwopengl.painter

import android.content.Context
import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext
import androidx.core.content.ContextCompat
import me.ztiany.androidav.opengl.jwopengl.common.EGLBridger
import me.ztiany.androidav.opengl.jwopengl.common.GLPainter
import me.ztiany.androidav.opengl.jwopengl.common.GLTexture
import me.ztiany.androidav.opengl.jwopengl.common.generateTexture
import timber.log.Timber

class RecorderShowPainter(
    private val context: Context,
    private val eglBridger: EGLBridger
) : GLPainter {

    /**承载视频的纹理*/
    private lateinit var cameraSurfaceTexture: SurfaceTexture
    private lateinit var cameraTexture: GLTexture
    private var onSurfaceText: ((SurfaceTexture) -> Unit)? = null

    private val soulFilter = SoulFilter()
    private val screenFilter = ScreenFilter()

    fun getSurfaceTexture(onSurfaceText: (SurfaceTexture) -> Unit) {
        if (::cameraSurfaceTexture.isInitialized) {
            onSurfaceText(cameraSurfaceTexture)
        } else {
            this.onSurfaceText = onSurfaceText
        }
    }

    override fun onSurfaceCreated() {
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

    override fun onDrawFrame() {
        cameraSurfaceTexture.updateTexImage()
        val glTexture = soulFilter.onDrawFrame(cameraTexture)
        screenFilter.onDrawFrame(glTexture)
    }

    override fun onSurfaceDestroy() {

    }

    private fun onFrameAvailable(surfaceTexture: SurfaceTexture) {
        eglBridger.requestRender()
    }

    fun setVideoAttribute(width: Int, height: Int, displayOrientation: Int) {
        Timber.d("setVideoAttribute")
        soulFilter.setTextureAttribute(width, height, displayOrientation)
        screenFilter.setTextureAttribute(width, height, displayOrientation)
    }

}