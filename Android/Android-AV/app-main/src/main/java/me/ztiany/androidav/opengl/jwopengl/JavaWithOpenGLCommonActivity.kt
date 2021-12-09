package me.ztiany.androidav.opengl.jwopengl

import android.content.Context
import android.content.Intent
import android.opengl.GLSurfaceView
import me.ztiany.androidav.databinding.OpenglActivityCommonBinding
import me.ztiany.androidav.opengl.jwopengl.common.CompoundRenderer
import me.ztiany.androidav.opengl.jwopengl.common.GLController
import me.ztiany.androidav.opengl.jwopengl.common.GLPainter
import me.ztiany.androidav.opengl.jwopengl.common.GLParams
import me.ztiany.lib.avbase.BaseActivity
import timber.log.Timber

class JavaWithOpenGLCommonActivity : BaseActivity<OpenglActivityCommonBinding>() {

    companion object {

        private const val KEY_DEFAULT_PAINTER = "key_default_painter"
        private const val KEY_TITLE = "key_title"
        private const val KEY_CONTROLLER = "key_controller"
        private const val KEY_PARAMS = "key_params"

        fun start(
            context: Context,
            title: String,
            defaultPainter: Class<out GLPainter>,
            controller: Class<out GLController>? = null,
            params: GLParams? = null
        ) {

            with(Intent(context, JavaWithOpenGLCommonActivity::class.java)) {
                putExtra(KEY_TITLE, title)
                putExtra(KEY_DEFAULT_PAINTER, defaultPainter)
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
        val defaultPainter = intent.getSerializableExtra(KEY_DEFAULT_PAINTER) as Class<*>
        val title = intent.getStringExtra(KEY_TITLE)

        //title
        supportActionBar?.title = title

        //renderer
        val renderer = CompoundRenderer(defaultPainter.newInstance() as GLPainter)
        binding.openglGlSurfaceView.setRenderer(renderer)
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