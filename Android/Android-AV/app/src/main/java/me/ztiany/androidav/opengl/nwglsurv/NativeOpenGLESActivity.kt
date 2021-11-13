package me.ztiany.androidav.opengl.nwglsurv

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.databinding.NativeOpenglActivityMainBinding
import me.ztiany.androidav.opengl.TemplateGLSurfaceView
import me.ztiany.androidav.utils.newMMLayoutParams

class NativeOpenGLESActivity : AppCompatActivity() {

    private lateinit var binding: NativeOpenglActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = NativeOpenglActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setUpView()
    }

    private fun setUpView() {
        binding.flRoot.addView(TemplateGLSurfaceView(this, NativeRenderer()), newMMLayoutParams())
    }

}