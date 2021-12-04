package me.ztiany.androidav.opengl.jwopengl.preview

import android.content.Context
import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.opengl.Matrix
import androidx.core.content.ContextCompat
import me.ztiany.androidav.opengl.jwopengl.common.*
import timber.log.Timber
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class CameraRenderer(
    private val context: Context,
    private val glSurfaceView: GLSurfaceView
) : GLSurfaceView.Renderer {

    private val glMVPMatrix = GLMVPMatrix()
    private lateinit var glProgram: GLProgram
    private lateinit var glTexture: GLTexture

    /**用于修正相机的方向*/
    private var displayOrientation = 0

    /**承载视频的纹理*/
    private lateinit var surfaceTexture: SurfaceTexture

    /**矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /**纹理坐标*/
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinate())

    private var onSurfaceText: ((SurfaceTexture) -> Unit)? = null

    fun getSurfaceTexture(onSurfaceText: (SurfaceTexture) -> Unit) {
        if (::surfaceTexture.isInitialized) {
            onSurfaceText(surfaceTexture)
        } else {
            this.onSurfaceText = onSurfaceText
        }
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        glProgram = GLProgram.fromAssets(
            "shader/vertex_mvp.glsl",
            "shader/fragment_camera.glsl"
        )

        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")
        glProgram.activeUniform("uTexture")
        glProgram.activeUniform("uMVPModelMatrix")

        glTexture = generateTexture(
            glProgram.uniformHandle("uTexture"),
            0,
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES)

        surfaceTexture = SurfaceTexture(glTexture.id)
        ContextCompat.getMainExecutor(context).execute {
            onSurfaceText?.invoke(surfaceTexture)
            onSurfaceText = null
        }
        surfaceTexture.setOnFrameAvailableListener {
            onFrameAvailable(it)
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        Timber.d("onSurfaceChanged")
        GLES20.glViewport(0, 0, width, height)
        glMVPMatrix.setWorldSize(width, height)
        adjustMatrix()
    }

    override fun onDrawFrame(gl: GL10?) {
        glProgram.startDraw {
            clearColorBuffer()
            glTexture.activeTexture()
            surfaceTexture.updateTexImage()
            uniformMatrix4fv("uMVPModelMatrix", glMVPMatrix.mvpMatrix)
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            drawArraysStrip(4/*4 个顶点*/)
        }
    }

    private fun onFrameAvailable(surfaceTexture: SurfaceTexture) {
        glSurfaceView.requestRender()
    }

    fun setVideoAttribute(width: Int, height: Int, displayOrientation: Int) {
        Timber.d("setVideoAttribute")
        this.displayOrientation = displayOrientation
        glMVPMatrix.setModelSize(width, height)
        adjustMatrix()
    }

    private fun adjustMatrix() {
        glMVPMatrix.lookAtNormally()
        glMVPMatrix.adjustToOrthogonal()
        glMVPMatrix.combineMVP()
        //绕着 Z 轴旋转
        Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, -this.displayOrientation.toFloat(), 0F, 0F, 1F)
    }

}