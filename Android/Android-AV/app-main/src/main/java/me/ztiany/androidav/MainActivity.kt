package me.ztiany.androidav

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.yanzhenjie.permission.AndPermission
import com.yanzhenjie.permission.runtime.Permission
import me.ztiany.androidav.avapi.MediaApiActivity
import me.ztiany.androidav.avtest.ToolsActivity
import me.ztiany.androidav.camera.CameraActivity
import me.ztiany.androidav.databinding.ActivityMainBinding
import me.ztiany.androidav.ffmpeg.FFmpegActivity
import me.ztiany.androidav.opengl.OpenGLESMainActivity
import me.ztiany.androidav.player.PlayerMainActivity
import me.ztiany.androidav.stream.StreamingMediaActivity
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
        binding.btnAvTest.setOnClickListener {
            startActivity(Intent(this, ToolsActivity::class.java))
        }

        binding.btnAvOperation.setOnClickListener {
            startActivity(Intent(this, MediaApiActivity::class.java))
        }

        binding.btnCamera.setOnClickListener {
            startActivity(Intent(this, CameraActivity::class.java))
        }

        binding.btnPlayer.setOnClickListener {
            startActivity(Intent(this, PlayerMainActivity::class.java))
        }

        binding.btnOpenGL.setOnClickListener {
            startActivity(Intent(this, OpenGLESMainActivity::class.java))
        }

        binding.btnFfmpeg.setOnClickListener {
            startActivity(Intent(this, FFmpegActivity::class.java))
        }

        binding.btnStreamingMedia.setOnClickListener {
            startActivity(Intent(this, StreamingMediaActivity::class.java))
        }
    }

    private fun requestAllPermissions() {
        AndPermission.with(this)
            .runtime()
            .permission(
                Permission.Group.STORAGE,
                Permission.Group.CAMERA,
                Permission.Group.MICROPHONE,
            )
            .onDenied {
                supportFinishAfterTransition()
            }
            .start()
    }

}