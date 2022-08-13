package me.ztiany.androidav.avapi.audio.opensles

import android.net.Uri
import android.os.Bundle
import com.android.base.delegate.simpl.DelegateActivity
import com.android.sdk.mediaselector.common.ResultListener
import com.android.sdk.mediaselector.system.newSystemMediaSelector
import com.blankj.utilcode.util.UriUtils
import me.ztiany.androidav.databinding.AudioActivityOpenslesBinding
import me.ztiany.lib.avbase.utils.printMediaTrackInfo
import timber.log.Timber

class OpenSLESActivity : DelegateActivity() {

    private var selectedFile: Uri? = null

    private val openSlES by lazy { OpenSlES(this) }

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

    private lateinit var binding: AudioActivityOpenslesBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = AudioActivityOpenslesBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpViews()
    }

    private fun setUpViews() {
        binding.audioSelectAudio.setOnClickListener {
            systemMediaSelector.takeFileFromSystem().mimeType("*/*").start()
        }

        binding.audioPrintTracks.setOnClickListener {
            selectedFile?.let {
                printMediaTrackInfo(this, it)
            }
        }

        binding.audioPlay.setOnClickListener {
            selectedFile?.let {
                startByFile(it)
            }
        }

        binding.audioPause.setOnClickListener {
            selectedFile?.let {
                pauseByFile(it)
            }
        }

        binding.audioResume.setOnClickListener {
            selectedFile?.let {
                resumeByFile(it)
            }
        }

        binding.audioStop.setOnClickListener {
            selectedFile?.let {
                stopByFile(it)
            }
        }
    }


    private fun startByFile(uri: Uri) {
        val path = UriUtils.uri2File(uri).absolutePath
        if (path.endsWith(".pcm")) {
            /* make sure you have the right config.*/
            openSlES.createPCMPlayer(44100, 1, 16)
            openSlES.startPcmPlayer(path)
        }
    }

    private fun pauseByFile(uri: Uri) {
        val path = UriUtils.uri2File(uri).absolutePath
        if (path.endsWith(".pcm")) {
            openSlES.pausePcmPlayer()
        }
    }

    private fun resumeByFile(uri: Uri) {
        val path = UriUtils.uri2File(uri).absolutePath
        if (path.endsWith(".pcm")) {
            openSlES.resumePcmPlayer()
        }
    }

    private fun stopByFile(uri: Uri) {
        val path = UriUtils.uri2File(uri).absolutePath
        if (path.endsWith(".pcm")) {
            openSlES.stopPcmPlayer()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        openSlES.stopPcmPlayer()
    }

}