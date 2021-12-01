package me.ztiany.rtmp

import android.content.Context
import android.content.Intent
import android.media.projection.MediaProjection
import android.media.projection.MediaProjectionManager
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.yanzhenjie.permission.AndPermission
import com.yanzhenjie.permission.runtime.Permission
import me.ztiany.rtmp.encoder.ScreenH264Encoder
import me.ztiany.rtmp.livevideo.MediaPusher
import me.ztiany.rtmp.source.VideoConfig
import me.ztiany.rtmp.source.screen.ScreenVideoSource


class MainActivity : AppCompatActivity() {

    private var mediaProjectionManager: MediaProjectionManager? = null

    private var liveManager: LiveManager? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        askPermissions()
    }

    private fun askPermissions() {
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

    fun liveScreen(view: View) {
        MediaPusher()

        this.mediaProjectionManager = getSystemService(Context.MEDIA_PROJECTION_SERVICE) as MediaProjectionManager
        mediaProjectionManager?.run {
            startActivityForResult(createScreenCaptureIntent(), 100)
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == 100 && resultCode == RESULT_OK && data != null) {
            mediaProjectionManager?.run {
                val mediaProjection = getMediaProjection(resultCode, data)
                startScreenLive(mediaProjection)
            }
        }
    }

    private fun startScreenLive(mediaProjection: MediaProjection) {
        val liveManager = LiveManager(
            VideoConfig(720, 1280),
            ScreenVideoSource(mediaProjection),
            ScreenH264Encoder()
        )

        liveManager.start()

        this.liveManager = liveManager
    }

}