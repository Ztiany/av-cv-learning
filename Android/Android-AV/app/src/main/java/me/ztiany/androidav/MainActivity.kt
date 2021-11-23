package me.ztiany.androidav

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.yanzhenjie.permission.AndPermission
import com.yanzhenjie.permission.runtime.Permission
import me.ztiany.androidav.camera.CameraActivity
import me.ztiany.androidav.databinding.ActivityMainBinding
import me.ztiany.androidav.opengl.OpenGLESActivity
import me.ztiany.androidav.video.VideoActivity
import timber.log.Timber

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Timber.d("Android-AV Started")
        requestAllPermissions()
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpView()
    }

    private fun setUpView() {
        binding.btnInit.setOnClickListener {
            System.loadLibrary("androidav")
        }

        binding.btnOpengl.setOnClickListener {
            startActivity(Intent(this, OpenGLESActivity::class.java))
        }

        binding.btnCamera.setOnClickListener {
            startActivity(Intent(this, CameraActivity::class.java))
        }

        binding.btnVideo.setOnClickListener {
            startActivity(Intent(this, VideoActivity::class.java))
        }
    }

    private fun requestAllPermissions() {
        AndPermission.with(this)
            .runtime()
            .permission(
                Permission.Group.STORAGE,
                Permission.Group.CAMERA,
                Permission.Group.MICROPHONE
            )
            .onDenied {
                supportFinishAfterTransition()
            }
            .start()
    }

}