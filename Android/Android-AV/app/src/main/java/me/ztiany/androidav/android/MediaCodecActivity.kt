package me.ztiany.androidav.android

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.android.h265.livescreen.receiver.ScreenReceiverActivity
import me.ztiany.androidav.android.h265.livescreen.sender.ScreenSenderActivity
import me.ztiany.androidav.databinding.MediacodecActivityMainBinding

class MediaCodecActivity : AppCompatActivity() {

    private lateinit var binding: MediacodecActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = MediacodecActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpView()
    }

    private fun setUpView() {
        binding.mediaCodecProjectScreenReceiver.setOnClickListener {
            startActivity(Intent(this, ScreenReceiverActivity::class.java))
        }

        binding.mediaCodecProjectScreenSender.setOnClickListener {
            startActivity(Intent(this, ScreenSenderActivity::class.java))
        }
    }

}