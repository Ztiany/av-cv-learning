package me.ztiany.androidav

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.yanzhenjie.permission.Action
import me.ztiany.androidav.databinding.ActivityMainBinding
import me.ztiany.androidav.opengl.nwglsurv.NativeWithGlsrvMainActivity
import com.yanzhenjie.permission.AndPermission
import com.yanzhenjie.permission.runtime.Permission
import me.ztiany.androidav.android.CameraActivity
import me.ztiany.androidav.android.MediaCodecActivity
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
            startActivity(Intent(this, NativeWithGlsrvMainActivity::class.java))
        }

        binding.btnCamera.setOnClickListener {
            startActivity(Intent(this, CameraActivity::class.java))
        }

        binding.btnMediaCodec.setOnClickListener {
            startActivity(Intent(this, MediaCodecActivity::class.java))
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