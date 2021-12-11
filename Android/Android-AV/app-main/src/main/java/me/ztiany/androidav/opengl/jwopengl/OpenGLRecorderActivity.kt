package me.ztiany.androidav.opengl.jwopengl

import android.graphics.Point
import android.util.Size
import android.widget.Toast
import androidx.core.view.doOnLayout
import me.ztiany.androidav.common.Directory
import me.ztiany.androidav.databinding.OpenglActivityRecorderBinding
import me.ztiany.androidav.opengl.jwopengl.recorder.RecorderManager
import me.ztiany.androidav.opengl.jwopengl.recorder.Speed
import me.ztiany.androidav.opengl.jwopengl.recorder.getSpeedName
import me.ztiany.androidav.opengl.oglcamera.CameraBuilder
import me.ztiany.androidav.opengl.oglcamera.CameraListener
import me.ztiany.androidav.opengl.oglcamera.CameraOperator
import me.ztiany.lib.avbase.BaseActivity
import java.io.File

/**
 * TODO:
 * 1. 解决偶尔黑屏的 Bug。
 * 2. 抽离出 FBO，封装出更多的 Shader。
 */
class OpenGLRecorderActivity : BaseActivity<OpenglActivityRecorderBinding>() {

    private var cameraOperator: CameraOperator? = null
    private val recorderManager = RecorderManager(this)
    private var storePath: String = ""
    private var currentSpeed = Speed.MODE_NORMAL

    private fun onCameraAvailable(previewSize: Size, displayOrientation: Int, isFront: Boolean) {
        recorderManager.onCameraAvailable(previewSize, displayOrientation, isFront) {
            cameraOperator?.startPreview(it)
        }
    }

    override fun setUpView() {
        recorderManager.init(binding.openglCameraView)
        setUpRecordBtnLogic()
        setUpSpeedBtnLogic()
        binding.openglBtnSwitch.setOnClickListener {
            cameraOperator?.switchCamera()
        }
        window.decorView.doOnLayout {
            setUpCamera()
        }
    }

    private fun setUpSpeedBtnLogic() {
        recorderManager.speed = currentSpeed
        binding.openglBtnSpeed.text = currentSpeed.getSpeedName()
        binding.openglBtnSpeed.setOnClickListener {
            var next = currentSpeed.ordinal + 1
            if (next > Speed.MODE_EXTRA_FAST.ordinal) {
                next = 0
            }
            currentSpeed = Speed.values()[next]
            binding.openglBtnSpeed.text = currentSpeed.getSpeedName()
            recorderManager.speed = currentSpeed
        }
    }

    private fun setUpRecordBtnLogic() {
        binding.openglBtnStart.setOnClickListener {
            if (recorderManager.isRecording) {
                recorderManager.stopRecording()
                Toast.makeText(this, "视频保存到了 $storePath", Toast.LENGTH_LONG).show()
                binding.openglBtnStart.text = "开始"
            } else {
                storePath = File(Directory.getSDCardRootPath(), Directory.createTempFileName(Directory.VIDEO_FORMAT_MP4)).absolutePath
                recorderManager.startRecording(storePath)
                binding.openglBtnStart.text = "停止"
            }

            binding.openglBtnSwitch.isEnabled = !recorderManager.isRecording
            binding.openglBtnSpeed.isEnabled = !recorderManager.isRecording
        }
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
            .previewViewSize(Point(openglCameraView.width, openglCameraView.height))
            .targetPreviewSize(Point(800, 480))
            .specificCameraId(CameraOperator.CAMERA_ID_BACK)
            .context(applicationContext)
            .rotation(windowManager.defaultDisplay.rotation)
            .build("2")
            .also {
                it.start()
            }
    }

    override fun onResume() {
        super.onResume()
        cameraOperator?.start()
    }

    override fun onPause() {
        super.onPause()
        cameraOperator?.stop()
        recorderManager.stopRecording()
    }

    override fun onDestroy() {
        super.onDestroy()
        cameraOperator?.release()
    }

}