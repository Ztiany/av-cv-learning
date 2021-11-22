package me.ztiany.androidav.android

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.android.camera1.Camera1BackActivity
import me.ztiany.androidav.databinding.CameraActivityMainBinding

class CameraActivity : AppCompatActivity() {

    private lateinit var binding: CameraActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = CameraActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setUpView()
    }

    private fun setUpView() {
        binding.cameraApi1Back.setOnClickListener {
            startActivity(Intent(this, Camera1BackActivity::class.java))
        }
    }


}