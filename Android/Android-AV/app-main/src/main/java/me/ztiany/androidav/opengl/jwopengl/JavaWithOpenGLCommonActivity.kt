package me.ztiany.androidav.opengl.jwopengl

import android.content.Context
import android.content.Intent
import android.opengl.GLSurfaceView
import me.ztiany.androidav.databinding.OpenglActivityCommonBinding
import me.ztiany.lib.avbase.BaseActivity
import timber.log.Timber

class JavaWithOpenGLCommonActivity : BaseActivity<OpenglActivityCommonBinding>() {

    companion object {

        private const val KEY_RENDERER = "key_renderer"
        private const val KEY_CONTROLLER = "key_controller"
        private const val KEY_PARAMS = "key_params"

        fun start(
            context: Context,
            renderer: Class<out GLSurfaceView.Renderer>,
            controller: Class<out GLController>? = null,
            params: GLParams? = null
        ) {

            with(Intent(context, JavaWithOpenGLCommonActivity::class.java)) {
                putExtra(KEY_RENDERER, renderer)
                putExtra(KEY_CONTROLLER, controller)
                putExtra(KEY_PARAMS, params)
                context.startActivity(this)
            }
        }

    }

    override fun setUpView() {
        initGlSurfaceView()
        setUpGlSurfaceView()
    }

    private fun initGlSurfaceView() {
        Timber.d("${binding.openglGlSurfaceView}")
        binding.openglGlSurfaceView.setEGLContextClientVersion(2)
    }

    private fun setUpGlSurfaceView() {
        val renderer = intent.getSerializableExtra(KEY_RENDERER) as Class<out GLSurfaceView.Renderer>
        binding.openglGlSurfaceView.setRenderer(renderer.newInstance())
        binding.openglGlSurfaceView.renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
    }

    override fun onResume() {
        super.onResume()
        binding.openglGlSurfaceView.onResume()
    }

    override fun onPause() {
        super.onPause()
        binding.openglGlSurfaceView.onPause()
    }

}