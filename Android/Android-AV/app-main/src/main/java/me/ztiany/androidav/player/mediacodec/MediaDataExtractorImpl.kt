package me.ztiany.androidav.player.mediacodec

import android.content.Context
import android.media.MediaExtractor
import android.media.MediaFormat
import android.net.Uri
import timber.log.Timber
import java.nio.ByteBuffer

class MediaDataExtractorImpl(private val context: Context) : MediaDataExtractor {

    private var mediaExtractor: MediaExtractor? = null

    private var format: MediaFormat? = null

    private val bufferInfo: BufferInfo = BufferInfo(0, 0, -1)
    private val bufferInfoCopy: BufferInfo = BufferInfo(0, 0, -1)

    private lateinit var selector: TrackSelector
    private lateinit var source: Uri

    override fun setSource(source: Uri) {
        this.source = source
    }

    override fun setTrackSelector(selector: TrackSelector) {
        this.selector = selector
    }

    override fun initExtractor() {
        if (!::source.isInitialized) {
            throw IllegalStateException("call setSource first.")
        }

        if (!::selector.isInitialized) {
            throw IllegalStateException("cal setTrackSelector first. ")
        }

        val extractor = MediaExtractor().apply {
            setDataSource(context, source, null)
            mediaExtractor = this
        }

        val trackCount = extractor.trackCount
        var mediaFormat: MediaFormat
        for (track in 0 until trackCount) {
            mediaFormat = extractor.getTrackFormat(track)
            if (selector(mediaFormat)) {
                bufferInfo.track = track
                format = mediaFormat
                extractor.selectTrack(track)
                Timber.d("MediaDataExtractorImpl select: $mediaFormat .")
                break
            }
        }

        if (bufferInfo.track == -1) {
            throw IllegalStateException("no tract satisfied.")
        }
    }

    override fun read(buffer: ByteBuffer): Int {
        val extractor = mediaExtractor ?: throw IllegalStateException("call start first.")
        buffer.clear()
        val readSize = extractor.readSampleData(buffer, 0)
        if (readSize < 0) {
            return -1
        }
        bufferInfo.sampleTime = extractor.sampleTime
        bufferInfo.sampleFlags = extractor.sampleFlags
        extractor.advance()
        return readSize
    }

    override fun release() {
        mediaExtractor?.release()
        mediaExtractor = null
        format = null
    }

    override fun getCurrentBufferInfo(): BufferInfo {
        bufferInfoCopy.track = bufferInfo.track
        bufferInfoCopy.sampleFlags = bufferInfo.sampleFlags
        bufferInfoCopy.sampleTime = bufferInfo.sampleTime
        return bufferInfoCopy
    }

    override fun seek(position: Long): Long {
        val extractor = mediaExtractor ?: throw IllegalStateException("call start first.")
        extractor.seekTo(position, MediaExtractor.SEEK_TO_PREVIOUS_SYNC)
        return extractor.sampleTime
    }

    override fun getMediaFormat(): MediaFormat {
        return format ?: throw IllegalStateException("MediaDataExtractorImpl is not working.")
    }

}