package me.ztiany.androidav

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.yanzhenjie.permission.AndPermission
import com.yanzhenjie.permission.runtime.Permission
import me.ztiany.androidav.audio.AudioActivity
import me.ztiany.androidav.camera.CameraActivity
import me.ztiany.androidav.common.printMediaCodecInfo
import me.ztiany.androidav.databinding.ActivityMainBinding
import me.ztiany.androidav.opengl.jwopengl.JavaWithOpenGLMainActivity
import me.ztiany.androidav.opengl.nwopengl.NativeWithOpenGLMainActivity
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
        printAllMediaInformation()
    }

    private fun setUpView() {
        binding.btnInit.setOnClickListener {
            System.loadLibrary("androidav")
        }

        binding.btnOpenglJava.setOnClickListener {
            startActivity(Intent(this, JavaWithOpenGLMainActivity::class.java))
        }

        binding.btnOpenglNative.setOnClickListener {
            startActivity(Intent(this, NativeWithOpenGLMainActivity::class.java))
        }

        binding.btnCamera.setOnClickListener {
            startActivity(Intent(this, CameraActivity::class.java))
        }

        binding.btnAudio.setOnClickListener {
            startActivity(Intent(this, AudioActivity::class.java))
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

    private fun printAllMediaInformation() {
        printMediaCodecInfo()
    }

}