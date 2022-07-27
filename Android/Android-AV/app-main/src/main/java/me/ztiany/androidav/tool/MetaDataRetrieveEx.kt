package me.ztiany.androidav.tool

import android.content.Context
import android.media.MediaMetadataRetriever
import android.net.Uri
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

suspend fun loadMediaMetadata(context: Context, uri: Uri): MediaMetadata {
    return MediaMetadataRetriever().let {
        it.setDataSource(context, uri)
        it.load()
    }
}

suspend fun loadMediaMetadata(path: String): MediaMetadata {
    return MediaMetadataRetriever().let {
        it.setDataSource(path)
        it.load()
    }
}


data class MediaMetadata(
    val rotation: Int,
    val duration: Int,
    val bitRate: Int,
    val width: Int,
    val height: Int
)

private suspend fun MediaMetadataRetriever.load(): MediaMetadata {
    return withContext(Dispatchers.IO) {
        val mediaMetadata = MediaMetadata(
            rotation = getInteger(extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION)),
            duration = getInteger(extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION)),
            bitRate = getInteger(extractMetadata(MediaMetadataRetriever.METADATA_KEY_BITRATE)),
            width = getInteger(extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH)),
            height = getInteger(extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT)),
        )
        release()
        mediaMetadata
    }
}

private fun getInteger(value: String?): Int {
    return if (value.isNullOrEmpty()) 0 else Integer.valueOf(value)
}