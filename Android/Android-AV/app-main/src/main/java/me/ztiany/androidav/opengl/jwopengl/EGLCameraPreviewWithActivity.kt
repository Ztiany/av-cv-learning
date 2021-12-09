package me.ztiany.androidav.opengl.jwopengl

import android.graphics.Point
import android.os.Bundle
import android.util.Size
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.R
import me.ztiany.androidav.opengl.jwopengl.common.EGLBridger
import me.ztiany.androidav.opengl.jwopengl.egl.*
import me.ztiany.androidav.opengl.jwopengl.painter.CameraPainter
import me.ztiany.androidav.opengl.oglcamera.CameraBuilder
import me.ztiany.androidav.opengl.oglcamera.CameraListener
import me.ztiany.androidav.opengl.oglcamera.CameraOperator

class EGLCameraPreviewWithActivity : AppCompatActivity() {

    companion object {
        const val WITH_SURFACE_VIEW = 1
        const val WITH_TEXTURE_VIEW = 2
        const val RENDER_TYPE = "RENDER_TYPE"
    }

    private lateinit var eglEnvironment: EGLEnvironment
    private lateinit var cameraOperator: CameraOperator
    private lateinit var cameraPainter: CameraPainter

    private fun onCameraAvailable(previewSize: Size, displayOrientation: Int) {
        if ((displayOrientation / 90).mod(2) == 1) {
            cameraPainter.setVideoAttribute(previewSize.height, previewSize.width, displayOrientation)
        } else {
            cameraPainter.setVideoAttribute(previewSize.width, previewSize.height, displayOrientation)
        }
        cameraPainter.getSurfaceTexture {
            cameraOperator.startPreview(it)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (intent.getIntExtra(RENDER_TYPE, WITH_SURFACE_VIEW) == WITH_SURFACE_VIEW) {
            setContentView(R.layout.opengl_activity_egl_surv_preview)
            setUpGlSurfaceView(SurfaceViewProvider(findViewById(R.id.opengl_egl_surface_view)))
            setUpCamera(Point(0, 0))
        } else {
            setContentView(R.layout.opengl_activity_egl_texv_preview)
            setUpGlSurfaceView(TextureViewProvider(findViewById(R.id.opengl_egl_texture_view)))
            setUpCamera(Point(0, 0))
        }
    }

    private fun setUpGlSurfaceView(surfaceProvider: SurfaceProvider) {
        eglEnvironment = EGLEnvironment(surfaceProvider, EGLAttribute())
        eglEnvironment.renderMode = RenderMode.WhenReady
        cameraPainter = CameraPainter(this, object : EGLBridger {
            override fun requestRender() {
                eglEnvironment.requestRender()
            }
        })
        eglEnvironment.start(CompoundRenderer(cameraPainter))
    }

    private fun setUpCamera(point: Point) {
        val cameraListener = CameraListener { _, _, previewSize, displayOrientation, _ ->
            onCameraAvailable(previewSize, displayOrientation)
        }

        cameraOperator = CameraBuilder()
            .cameraListener(cameraListener)
            .maxPreviewSize(Point(1920, 1080))
            .minPreviewSize(Point(100, 100))
            .specificCameraId(CameraOperator.CAMERA_ID_BACK)
            .context(applicationContext)
            .previewViewSize(point)
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