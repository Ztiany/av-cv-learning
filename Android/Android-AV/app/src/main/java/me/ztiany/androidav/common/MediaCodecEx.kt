@file:JvmName("MediaCodecEx")

package me.ztiany.androidav.common

import android.media.MediaCodecList
import timber.log.Timber

fun printMediaCodecInfo() {
    Timber.i("------------------------------------------------------------------------------------------------------")
    Timber.i("ALL_CODECS:")
    for (codecInfo in MediaCodecList(MediaCodecList.ALL_CODECS).codecInfos) {
        Timber.i("${codecInfo.name}-${codecInfo.supportedTypes.contentToString()}")
    }
    Timber.i("------------------------------------------------------------------------------------------------------")
    Timber.i("REGULAR_CODECS:")
    for (codecInfo in MediaCodecList(MediaCodecList.REGULAR_CODECS).codecInfos) {
        Timber.i("${codecInfo.name}-${codecInfo.supportedTypes.contentToString()}")
    }
}