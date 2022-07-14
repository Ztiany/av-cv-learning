package me.ztiany.androidav.audio.mediaplayer

import android.media.MediaPlayer
import android.net.Uri
import android.os.Bundle
import com.android.base.delegate.simpl.DelegateActivity
import com.android.sdk.mediaselector.common.ResultListener
import com.android.sdk.mediaselector.system.newSystemMediaSelector
import me.ztiany.androidav.databinding.AudioActivityMpBinding
import timber.log.Timber
import java.io.IOException

class AudioMediaPlayerActivity : DelegateActivity() {

    private var selectedFile: Uri? = null

    private lateinit var binding: AudioActivityMpBinding

    private val systemMediaSelector by lazy {
        newSystemMediaSelector(this, object : ResultListener {
            override fun onTakeSuccess(result: List<Uri>) {
                Timber.d("onTakeSuccess: $result")
                selectedFile = result.getOrNull(0)
                showSelectedFile()
            }
        })
    }

    private fun showSelectedFile() {
        binding.audioSelectedFile.text = "已经选择：${selectedFile.toString()}"
    }

    private var mediaPlayer: MediaPlayer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = AudioActivityMpBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpView()
    }

    private fun setUpView() {
        binding.audioSelect.setOnClickListener {
            systemMediaSelector.takeFileFromSystem().mimeType("audio/*").start()
        }

        binding.audioPlay.setOnClickListener {
            stopPlay()
            selectedFile?.let {
                startPlaying(it)
            }
        }

        binding.audioStop.setOnClickListener {
            stopPlay()
        }
    }

    private fun startPlaying(uri: Uri) {
        mediaPlayer = MediaPlayer().apply {
            try {
                setDataSource(this@AudioMediaPlayerActivity, uri)
                prepare()
                start()
                setOnCompletionListener {
                    stopPlay()
                }
                Timber.d("startPlaying() succeeded")
            } catch (e: IOException) {
                Timber.e(e, "startPlaying() failed")
            }
        }
    }

    private fun stopPlay() {
        mediaPlayer?.apply {
            stop()
            release()
        }
        mediaPlayer = null
    }

    override fun onDestroy() {
        super.onDestroy()
        stopPlay()
    }

}