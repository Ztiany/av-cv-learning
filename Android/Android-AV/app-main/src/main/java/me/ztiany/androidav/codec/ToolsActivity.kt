package me.ztiany.androidav.codec

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.databinding.ToolsActivityBinding
import me.ztiany.lib.avbase.utils.printMediaCodecInfo

class ToolsActivity : AppCompatActivity() {

    private lateinit var binding: ToolsActivityBinding

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
            printCodecAbilities()
        }
    }

    private fun printCodecAbilities() {
        printMediaCodecInfo()
    }

}