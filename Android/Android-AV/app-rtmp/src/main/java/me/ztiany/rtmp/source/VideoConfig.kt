package me.ztiany.rtmp.source

import android.view.Surface

data class VideoConfig(
    val targetWidth: Int,
    val targetHeight: Int,
    var surface: Surface? = null
)