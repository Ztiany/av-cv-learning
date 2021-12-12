package me.ztiany.androidav.opengl.jwopengl.recorder

import android.media.*
import android.opengl.EGLContext
import android.opengl.GLES20
import android.opengl.Matrix
import android.view.Surface
import me.ztiany.androidav.opengl.jwopengl.common.GLRenderer
import me.ztiany.androidav.opengl.jwopengl.common.RenderMode
import me.ztiany.androidav.opengl.jwopengl.common.SurfaceProvider
import me.ztiany.androidav.opengl.jwopengl.common.SurfaceProviderCallback
import me.ztiany.androidav.opengl.jwopengl.egl14.*
import me.ztiany.androidav.opengl.jwopengl.gles2.*
import me.ztiany.androidav.opengl.jwopengl.recorder.encoder.Encoder
import me.ztiany.androidav.opengl.jwopengl.recorder.encoder.EncoderMode
import timber.log.Timber

/**渲染的是 FBO 中的纹理，使用标准的坐标系*/
class RecorderEncodeRenderer : GLRenderer {

    /**CPU 着色器程序*/
    private lateinit var glProgram: GLProgram

    /**用于修正坐标位置*/
    private val glMVPMatrix = GLMVPMatrix()

    /**矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /**纹理坐标*/
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateStandard())

    /**自定义的 EGL 环境*/
    private var eglEnvironment: EGLEnvironment? = null
    private var mediaCodecSurfaceProvider: MediaCodecSurfaceProvider? = null

    /**编码器*/
    private var encoder: Encoder? = null

    override fun onSurfaceCreated() {
        Timber.d("onSurfaceCreated() called")
        glProgram = GLProgram.fromAssets(
            "shader/vertex_mvp.glsl",
            "shader/fragment_texture.glsl"
        )

        //vertex
        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")
        glProgram.activeUniform("uMVPModelMatrix")

        //fragment
        glProgram.activeUniform("uTexture")
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        Timber.d("onSurfaceChanged() called with: width = $width, height = $height")
        GLES20.glViewport(0, 0, width, height)
    }

    override fun onDrawFrame(attachment: Any?) {
        val textureWithTime = attachment as? TextureWithTime ?: return
        glProgram.startDraw {
            //vertex
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            uniformMatrix4fv("uMVPModelMatrix", glMVPMatrix.mvpMatrix)
            //fragment
            //texture【将分享过来的纹理绘制到 FBO 的纹理上】
            textureWithTime.glTexture.activeTexture(uniformHandle("uTexture"))
            //draw
            drawArraysStrip(4/*4 个顶点*/)
            //setTime
            eglEnvironment?.setPresentationTime(textureWithTime.timestamp)
        }
    }

    override fun onSurfaceDestroy() {
        Timber.d("onSurfaceDestroy")
    }

    fun setVideoAttribute(attribute: TextureAttribute) {
        Timber.d("setVideoAttribute() called with: attribute = $attribute")
        if ((attribute.orientation / 90).mod(2) == 1) {//竖屏
            glMVPMatrix.setWorldSize(attribute.height, attribute.width)
            glMVPMatrix.setModelSize(attribute.height, attribute.width)
        } else {//横屏
            glMVPMatrix.setWorldSize(attribute.height, attribute.width)
            glMVPMatrix.setModelSize(attribute.width, attribute.height)
        }

        glMVPMatrix.resetToIdentity(glMVPMatrix.mvpMatrix)
        glMVPMatrix.lookAtNormally()
        glMVPMatrix.adjustToOrthogonal()
        glMVPMatrix.combineMVP()
        //绕着 Z 轴旋转
        if (!attribute.isFront) {
            //后摄，一般情况下相机的画面被逆时针转了 90 度，这是这里也将顶点坐标转同样的角度。
            //注意【顶点是先插值，然后我们利用矩阵再将顶点修正到正确的采样进行位置】。
            Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, -attribute.orientation.toFloat(), 0F, 0F, 1F)
        } else {
            Matrix.scaleM(glMVPMatrix.mvpMatrix, 0, -1F, 1F, 1F)
            Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, attribute.orientation.toFloat(), 0F, 0F, 1F)
        }
    }

    fun start(sharedEGLContext: EGLContext, encoder: Encoder) {
        if (this.encoder != null) {
            throw UnsupportedOperationException("Already started..")
        }

        this.encoder = encoder
        encoder.init(glMVPMatrix.getModelWidth(), glMVPMatrix.getModelHeight())

        if (encoder.mode == EncoderMode.Hard) {
            startWithHardEncoder(sharedEGLContext, encoder)
        } else {
            throw UnsupportedOperationException("Not supported.")
        }
    }

    private fun startWithHardEncoder(sharedEGLContext: EGLContext, encoder: Encoder) {
        encoder.start()
        val surfaceProvider = MediaCodecSurfaceProvider(encoder.getInputSurfaceView())
        eglEnvironment = EGLEnvironment(
            surfaceProvider,
            EGLAttribute(sharedEGLContext)
        ).apply {
            renderMode = RenderMode.WhenDirty
            start(this@RecorderEncodeRenderer)
        }
        this.mediaCodecSurfaceProvider = surfaceProvider
    }

    fun stop() {
        mediaCodecSurfaceProvider?.stop()
        eglEnvironment?.release()
        encoder?.stop()
        eglEnvironment = null
        mediaCodecSurfaceProvider = null
        encoder = null
    }

    fun onFrame(frame: TextureWithTime) {
        eglEnvironment?.requestRender(frame)
    }

    private inner class MediaCodecSurfaceProvider(private val surface: Surface) : SurfaceProvider {

        private lateinit var surfaceProviderCallback: SurfaceProviderCallback

        override fun start(surfaceProviderCallback: SurfaceProviderCallback) {
            if (this::surfaceProviderCallback.isInitialized) {
                throw UnsupportedOperationException("call this only once.")
            }
            this.surfaceProviderCallback = surfaceProviderCallback
            surfaceProviderCallback.onSurfaceAvailable(surface)
            surfaceProviderCallback.onSurfaceChanged(surface, glMVPMatrix.getModelWidth(), glMVPMatrix.getModelHeight())
        }

        override fun stop() {
            surfaceProviderCallback.onSurfaceDestroyed()
        }
    }

}