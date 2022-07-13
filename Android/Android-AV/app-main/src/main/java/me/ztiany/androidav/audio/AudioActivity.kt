package me.ztiany.androidav.audio

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.audio.audiorecord.AudioRecordActivity
import me.ztiany.androidav.audio.autiotrack.AudioTrackActivity
import me.ztiany.androidav.audio.mixing.MixingAudioActivity
import me.ztiany.androidav.databinding.AudioActivityMainBinding

class AudioActivity : AppCompatActivity() {

    private lateinit var binding: AudioActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = AudioActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpView()
    }

    private fun setUpView() {
        //AudioRecord 录制 WAV
        binding.audioRecordByAr.setOnClickListener {
            startActivity(Intent(this, AudioRecordActivity::class.java))
        }

        //AudioTrack 播放 WAV
        binding.audioPlayByAt.setOnClickListener {
            startActivity(Intent(this, AudioTrackActivity::class.java))
        }

        //混音
        binding.audioMixingAudio.setOnClickListener {
            startActivity(Intent(this, MixingAudioActivity::class.java))
        }
    }

}