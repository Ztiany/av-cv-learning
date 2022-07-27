package me.ztiany.androidav.tool

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

        }
    }

    private fun printCodecAbilities() {
        printMediaCodecInfo()
    }

}