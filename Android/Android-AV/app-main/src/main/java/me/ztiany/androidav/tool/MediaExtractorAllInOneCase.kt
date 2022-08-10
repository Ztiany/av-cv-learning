package me.ztiany.androidav.tool

import android.content.Context
import android.media.MediaExtractor
import android.media.MediaFormat
import android.net.Uri
import me.ztiany.androidav.player.mediacodec.AUDIO_SELECTOR
import me.ztiany.androidav.player.mediacodec.VIDEO_SELECTOR
import timber.log.Timber
import java.nio.ByteBuffer
import kotlin.concurrent.thread

class MediaExtractorMultiThreadingCase(
    private val context: Context,
    private val source: Uri,
) : TestCase {


    private lateinit var mediaExtractor: MediaExtractor

    private var audioTrack = -1
    private var videoTrack = -1

    @Volatile private var isTesting = false

    override fun start() {
        isTesting = true
        initMediaExtractor()
        startReader(audioTrack, 5, "Audio")
        //startReader(videoTrack, "Video")
    }

    private fun startReader(track: Int, readTime: Int, flag: String) {
        val buffer = ByteBuffer.allocate(1024 * 1024 * 5)
        thread {
            var count = 0
            while (isTesting) {
                Timber.d("before read $flag, sampleTime = ${mediaExtractor.sampleTime}, sampleTrackIndex = ${mediaExtractor.sampleTrackIndex}")
                mediaExtractor.selectTrack(track)
                val readSize = mediaExtractor.readSampleData(buffer, 0)
                Timber.d("after read $flag, readSize = $readSize,  sampleTime = ${mediaExtractor.sampleTime}, sampleTrackIndex = ${mediaExtractor.sampleTrackIndex}")
                if (++count > readTime) {
                    break
                }
            }
        }
    }

    private fun initMediaExtractor() {
        mediaExtractor = MediaExtractor().apply {
            setDataSource(context, source, null)
            val trackCount = trackCount
            var tempFormat: MediaFormat?
            for (track in 0 until trackCount) {
                tempFormat = getTrackFormat(track)
                if (AUDIO_SELECTOR(tempFormat)) {
                    audioTrack = track
                    Timber.d("select: $tempFormat as AudioTrack.")
                } else if (VIDEO_SELECTOR(tempFormat)) {
                    videoTrack = track
                    Timber.d(" select: $tempFormat as VideoTrack.")
                }
                if (videoTrack != -1 && audioTrack != -1) {
                    break
                }
            }
        }
    }

    override fun stop() {
        isTesting = false
    }

}