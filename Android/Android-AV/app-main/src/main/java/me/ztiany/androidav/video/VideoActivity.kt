package me.ztiany.androidav.video

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.databinding.VideoActivityMainBinding
import me.ztiany.androidav.video.codec.livecamera.client.LiveCameraClientActivity
import me.ztiany.androidav.video.codec.livecamera.server.LiveCameraServerActivity
import me.ztiany.androidav.video.codec.livescreen.receiver.ScreenReceiverActivity
import me.ztiany.androidav.video.codec.livescreen.sender.ScreenSenderActivity
import me.ztiany.androidav.video.combine.VideoCombinationActivity
import me.ztiany.androidav.video.rendering.VideoMediaPlayerActivity

class VideoActivity : AppCompatActivity() {

    private lateinit var binding: VideoActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = VideoActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpView()
    }

    private fun setUpView() {
        //视频渲染
        binding.videoRenderByMp.setOnClickListener {
            startActivity(Intent(this, VideoMediaPlayerActivity::class.java))
        }

        //视频拼接
        binding.videoMosaic.setOnClickListener {
            startActivity(Intent(this, VideoCombinationActivity::class.java))
        }

        //投屏
        binding.videoH265ProjectScreenReceiver.setOnClickListener {
            startActivity(Intent(this, ScreenReceiverActivity::class.java))
        }
        binding.videoH265ProjectScreenSender.setOnClickListener {
            startActivity(Intent(this, ScreenSenderActivity::class.java))
        }

        //视频相互传输
        binding.videoH265ShareCameraServer.setOnClickListener {
            startActivity(Intent(this, LiveCameraServerActivity::class.java))
        }
        binding.videoH265ShareCameraClient.setOnClickListener {
            startActivity(Intent(this, LiveCameraClientActivity::class.java))
        }

    }

}