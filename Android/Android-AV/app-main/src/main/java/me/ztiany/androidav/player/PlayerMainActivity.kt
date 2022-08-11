package me.ztiany.androidav.player

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.databinding.ActivityMainBinding
import me.ztiany.androidav.databinding.PlayActivityMainBinding
import me.ztiany.androidav.player.mediaplayer.VideoMediaPlayerActivity
import timber.log.Timber

class PlayerMainActivity : AppCompatActivity() {

    private lateinit var binding: PlayActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = PlayActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpView()
    }

    fun setUpView() {
        //视频渲染
        binding.videoRenderByMp.setOnClickListener {
            startActivity(Intent(this, VideoMediaPlayerActivity::class.java))
        }
    }

}