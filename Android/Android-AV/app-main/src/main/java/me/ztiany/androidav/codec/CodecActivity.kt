package me.ztiany.androidav.codec

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.databinding.CodecActivityMainBinding
import me.ztiany.lib.avbase.utils.printMediaCodecInfo

class CodecActivity : AppCompatActivity() {

    private lateinit var binding: CodecActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = CodecActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpViews()
    }

    private fun setUpViews() {
        binding.btnCodecAbility.setOnClickListener {
            printCodecAbilities()
        }
    }

    private fun printCodecAbilities() {
       printMediaCodecInfo()
    }

}