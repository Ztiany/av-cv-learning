package me.ztiany.androidav.opengl.nwopengl

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.databinding.OpenglActivityNativeGlsurvMainBinding
import me.ztiany.androidav.common.newMMLayoutParams
import me.ztiany.androidav.opengl.nwopengl.renderer.NativeRenderer

class NativeWithOpenGLMainActivity : AppCompatActivity() {

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