package me.ztiany.androidav.opengl.jwopengl

import android.app.Activity
import android.content.Intent
import android.opengl.GLSurfaceView
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.common.IEntrance
import me.ztiany.androidav.common.buildLayoutEntrance
import me.ztiany.androidav.opengl.jwopengl.egl.EGLPreviewWithActivity
import me.ztiany.androidav.opengl.jwopengl.preview.OpenGLCameraPreviewActivity
import me.ztiany.androidav.opengl.jwopengl.renderer.*

class JavaWithOpenGLMainActivity : AppCompatActivity() {

    private data class CommonItem(
        override val title: String,
        val renderer: Class<out GLSurfaceView.Renderer>,
        val controller: Class<out GLController>? = null,
        val params: GLParams? = null
    ) : IEntrance

    private data class ActivityItem(
        override val title: String,
        val activity: Class<out Activity>,
        val onIntent: ((Intent) -> Unit)? = null,
    ) : IEntrance

    private val entrances = listOf(
        CommonItem("绘制背景", BackgroundRenderer::class.java),
        CommonItem("绘制三角形", TriangleRenderer::class.java),
        CommonItem("绘制渐变矩形", GradualRectangleRenderer::class.java),
        CommonItem("绘制纹理", TextureRenderer::class.java),
        CommonItem("绘制纹理（修正1）", Fixed1TextureRenderer::class.java),
        CommonItem("绘制纹理（修正2）", Fixed2TextureRenderer::class.java),
        ActivityItem("相机预览（GLSurfaceView）", OpenGLCameraPreviewActivity::class.java),
        ActivityItem("相机预览（EGL + SurfaceView）", EGLPreviewWithActivity::class.java) {
            it.putExtra(EGLPreviewWithActivity.RENDER_TYPE, EGLPreviewWithActivity.WITH_SURFACE_VIEW)
        },
        ActivityItem("相机预览（EGL + TextureView）", EGLPreviewWithActivity::class.java) {
            it.putExtra(EGLPreviewWithActivity.RENDER_TYPE, EGLPreviewWithActivity.WITH_TEXTURE_VIEW)
        },
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(
            buildLayoutEntrance(
                this,
                entrances
            ) { _, index ->
                handleClicked(index)
            }
        )
    }

    private fun handleClicked(index: Int) {
        val item = entrances[index]
        if (item is CommonItem) {
            JavaWithOpenGLCommonActivity.start(this, item.title, item.renderer)
        } else if (item is ActivityItem) {
            startActivity(Intent(this, item.activity).apply {
                item.onIntent?.invoke(this)
            })
        }
    }

}