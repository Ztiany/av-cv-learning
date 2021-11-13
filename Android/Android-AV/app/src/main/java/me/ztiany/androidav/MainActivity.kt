package me.ztiany.androidav

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import me.ztiany.androidav.databinding.ActivityMainBinding
import me.ztiany.androidav.opengl.nwglsurv.NativeOpenGLESActivity

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpView()
    }

    private fun setUpView() {
        binding.btnInit.setOnClickListener {
            System.loadLibrary("androidav")
        }

        binding.btnNativeOpengl.setOnClickListener {
            startActivity(Intent(this, NativeOpenGLESActivity::class.java))
        }

    }

}