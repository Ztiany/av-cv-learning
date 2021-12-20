package me.ztiany.rtmp

import android.content.Intent
import android.view.View
import com.blankj.utilcode.util.ToastUtils
import com.yanzhenjie.permission.AndPermission
import com.yanzhenjie.permission.runtime.Permission
import me.ztiany.lib.avbase.app.BaseActivity
import me.ztiany.rtmp.common.Pusher
import me.ztiany.rtmp.databinding.ActivityMainBinding
import me.ztiany.rtmp.practice.screen.ScreenPusher

private const val URL_SELF = "rtmp://139.198.183.182/live/livestream"

class MainActivity : BaseActivity<ActivityMainBinding>() {

    private var pusher: Pusher? = null

    override fun setUpView() {
        super.setUpView()
        askPermissions()
        binding.rtmpBtnScreenHard.setOnClickListener {
            liveScreenHard()
        }
        binding.rtmpBtnScreenSoft.setOnClickListener {
            ToastUtils.showLong("不支持")
        }
        binding.rtmpBtnCameraSoft.setOnClickListener {
            liveCameraSoft()
        }
        binding.rtmpBtnStop.setOnClickListener {
            pusher?.stop()
            showFunctions()
        }
    }

    private fun showFunctions() {
        binding.rtmpLlController.visibility = View.GONE
        binding.rtmpLlScreen.visibility = View.VISIBLE
        binding.rtmpLlCamera.visibility = View.VISIBLE
    }

    private fun showController() {
        binding.rtmpLlController.visibility = View.VISIBLE
        binding.rtmpLlScreen.visibility = View.GONE
        binding.rtmpLlCamera.visibility = View.GONE
    }

    private fun askPermissions() {
        AndPermission.with(this)
            .runtime()
            .permission(
                Permission.Group.STORAGE,
                Permission.Group.CAMERA,
                Permission.Group.MICROPHONE
            )
            .onDenied {
                supportFinishAfterTransition()
            }
            .start()
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        pusher?.onActivityResult(requestCode, resultCode, data)
    }

    private fun liveScreenHard() {
        //开始直播
        ScreenPusher(this).also {
            this.pusher = it
            it.start(URL_SELF)
        }
        //切换 UI
        showController()
    }

    private fun liveCameraSoft() {

        //切换 UI
        showController()
    }

    override fun onDestroy() {
        super.onDestroy()
        pusher?.stop()
    }

}