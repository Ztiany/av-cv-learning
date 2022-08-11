package me.ztiany.androidav.avtest

import android.net.Uri
import android.os.Bundle
import com.android.base.delegate.simpl.DelegateActivity
import com.android.sdk.mediaselector.common.ResultListener
import com.android.sdk.mediaselector.system.newSystemMediaSelector
import me.ztiany.androidav.databinding.ToolsActivityBinding
import me.ztiany.lib.avbase.utils.printMediaCodecInfo
import timber.log.Timber

class ToolsActivity : DelegateActivity() {

    private val systemMediaSelector by lazy {
        newSystemMediaSelector(this, object : ResultListener {
            override fun onTakeSuccess(result: List<Uri>) {
                Timber.d("onTakeSuccess: $result")
                result.getOrNull(0)?.let {
                    onMediaFileTaken?.invoke(it)
                }
            }
        })
    }

    private lateinit var binding: ToolsActivityBinding

    private var testCase: TestCase? = null

    private var onMediaFileTaken: ((Uri) -> Unit)? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ToolsActivityBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpViews()
    }

    private fun setUpViews() {
        binding.btnCodecAbility.setOnClickListener {
            printCodecAbilities()
        }

        binding.btnMetadataRetrieve.setOnClickListener {

        }

        binding.btnMediaExtractorMt.setOnClickListener {
            onMediaFileTaken = {
                testCase = MediaExtractorMultiThreadingCase(this, it).apply { start() }
            }
            systemMediaSelector.takeFileFromSystem().mimeType("video/*").start()
        }

        binding.btnMediaExtractorAio.setOnClickListener {
            onMediaFileTaken = {
                testCase = MediaExtractorAllInOneCase(this, it).apply { start() }
            }
            systemMediaSelector.takeFileFromSystem().mimeType("video/*").start()
        }
    }

    private fun printCodecAbilities() {
        printMediaCodecInfo()
    }

    override fun onStop() {
        super.onStop()
        testCase?.stop()
    }

}