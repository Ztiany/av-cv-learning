package me.ztiany.androidav.opengl.nwglsurv

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.databinding.OpenglActivityNativeGlsurvMainBinding
import me.ztiany.androidav.utils.newMMLayoutParams

class NativeWithGlsrvMainActivity : AppCompatActivity() {

    private lateinit var binding: OpenglActivityNativeGlsurvMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = OpenglActivityNativeGlsurvMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setUpView()
    }

    private fun setUpView() {
        binding.flRoot.addView(TemplateGLSurfaceView(this, NativeRenderer()), newMMLayoutParams())
    }

}