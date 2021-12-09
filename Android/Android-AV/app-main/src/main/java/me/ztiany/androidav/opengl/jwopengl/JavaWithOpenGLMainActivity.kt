package me.ztiany.androidav.opengl.jwopengl

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.common.IEntrance
import me.ztiany.androidav.common.buildLayoutEntrance
import me.ztiany.androidav.opengl.jwopengl.common.GLController
import me.ztiany.androidav.opengl.jwopengl.common.GLPainter
import me.ztiany.androidav.opengl.jwopengl.common.GLParams
import me.ztiany.androidav.opengl.jwopengl.painter.*

class JavaWithOpenGLMainActivity : AppCompatActivity() {

    private data class CommonItem(
        override val title: String,
        val defaultPainter: Class<out GLPainter>,
        val controller: Class<out GLController>? = null,
        val params: GLParams? = null
    ) : IEntrance

    private data class ActivityItem(
        override val title: String,
        val activity: Class<out Activity>,
        val onIntent: ((Intent) -> Unit)? = null,
    ) : IEntrance

    private val entrances = listOf(
        CommonItem("绘制背景", BackgroundPainter::class.java),
        CommonItem("绘制三角形", TrianglePainter::class.java),
        CommonItem("绘制渐变矩形", GradualRectanglePainter::class.java),
        CommonItem("绘制纹理", TexturePainter::class.java),
        CommonItem("绘制纹理（修正1）", Fixed1TexturePainter::class.java),
        CommonItem("绘制纹理（修正2）", Fixed2TexturePainter::class.java),
        ActivityItem("相机预览（GLSurfaceView）", OpenGLCameraPreviewActivity::class.java),
        ActivityItem("相机预览（EGL + SurfaceView）", EGLCameraPreviewWithActivity::class.java) {
            it.putExtra(EGLCameraPreviewWithActivity.RENDER_TYPE, EGLCameraPreviewWithActivity.WITH_SURFACE_VIEW)
        },
        ActivityItem("相机预览（EGL + TextureView）", EGLCameraPreviewWithActivity::class.java) {
            it.putExtra(EGLCameraPreviewWithActivity.RENDER_TYPE, EGLCameraPreviewWithActivity.WITH_TEXTURE_VIEW)
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
            JavaWithOpenGLCommonActivity.start(this, item.title, item.defaultPainter)
        } else if (item is ActivityItem) {
            startActivity(Intent(this, item.activity).apply {
                item.onIntent?.invoke(this)
            })
        }
    }

}