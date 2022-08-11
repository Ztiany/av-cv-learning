package me.ztiany.androidav.player.mediacodec

import android.content.Context
import android.media.MediaExtractor
import android.media.MediaFormat
import android.net.Uri
import kotlinx.coroutines.yield
import timber.log.Timber
import java.util.concurrent.ArrayBlockingQueue
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.concurrent.thread

class MediaDataExtractorImpl(
    private val context: Context,
    private val byteBufferPool: ByteBufferPool
) : MediaDataExtractor {

    private var mediaExtractor: MediaExtractor? = null

    private lateinit var source: Uri

    private var audioFormat: MediaFormat? = null
    private var audioTrack = -1

    private var videoFormat: MediaFormat? = null
    private var videoTrack = -1

    private var audioSelector: TrackSelector = AUDIO_SELECTOR
    private var videoSelector: TrackSelector = VIDEO_SELECTOR

    private val maxVideoPacketCount = 100
    private val yieldVideoPacketCount = 90
    private val maxAudioPacketCount = 50
    private val yieldAudioPacketCount = 45

    @Volatile private var seekTime = -1L

    private val started = AtomicBoolean(false)

    @Volatile private var audioReachEnd = false
    @Volatile private var videoReachEnd = false

    private var onPrepared: ((audioFormat: MediaFormat?, videoFormat: MediaFormat?) -> Unit)? = null

    private val audioPacketQueue = ArrayBlockingQueue<Packet>(maxVideoPacketCount)
    private val videoPacketQueue = ArrayBlockingQueue<Packet>(maxAudioPacketCount)

    override fun setSource(source: Uri) {
        this.source = source
    }

    override fun setAudioTrackSelector(selector: TrackSelector) {
        this.audioSelector = selector
    }

    override fun setVideoTrackSelector(selector: TrackSelector) {
        this.videoSelector = selector
    }

    override fun start() {
        if (started.compareAndSet(false, true)) {
            internalStart()
        }
    }

    override fun invokeOnPrepared(onPrepared: (audioFormat: MediaFormat?, videoFormat: MediaFormat?) -> Unit) {
        this.onPrepared = onPrepared
    }

    private fun internalStart() {
        thread {
            release()
            initExtractor()
            onPrepared?.invoke(audioFormat, videoFormat)
            if (audioFormat == null && videoFormat == null) {
                return@thread
            }
            while (started.get() && (!audioReachEnd && !videoReachEnd)) {
                handleSeek()
                checkIfNeedYield()
                readAudio()
                readVideo()
            }
            release()
        }
    }

    private fun checkIfNeedYield() {
        if (audioPacketQueue.size >= yieldAudioPacketCount && videoPacketQueue.size >= yieldVideoPacketCount) {
            Thread.yield()
        }
    }

    private fun readAudio() {
        if (audioReachEnd || audioTrack == -1 || audioPacketQueue.size >= maxAudioPacketCount) {
            return
        }
        readSamples(audioTrack, audioPacketQueue)
    }

    private fun readVideo() {
        if (videoReachEnd || videoTrack == -1 || videoPacketQueue.size >= maxVideoPacketCount) {
            return
        }
        readSamples(videoTrack, videoPacketQueue)
    }

    private fun readSamples(track: Int, queue: ArrayBlockingQueue<Packet>) {
        val extractor = mediaExtractor ?: throw IllegalStateException("never happen")
        extractor.selectTrack(track)
        val byteBuffer = byteBufferPool.getByteBuffer(extractor.sampleSize)
        val readSize = extractor.readSampleData(byteBuffer, 0)
        if (readSize < 0) {
            onTrackReachEnd(track)
            return
        }
        val packet = Packet(byteBuffer, readSize, extractor.sampleTime, extractor.sampleFlags)
        queue.put(packet)
        extractor.advance()
    }

    private fun onTrackReachEnd(track: Int) {
        if (track == audioTrack) {
            audioReachEnd = true
        } else if (track == videoTrack) {
            videoReachEnd = true
        }
    }

    private fun initExtractor() {
        if (!::source.isInitialized) {
            throw IllegalStateException("call setSource first.")
        }

        val extractor = MediaExtractor().apply {
            setDataSource(context, source, null)
            mediaExtractor = this
        }

        val trackCount = extractor.trackCount
        var mediaFormat: MediaFormat
        for (track in 0 until trackCount) {
            mediaFormat = extractor.getTrackFormat(track)
            if (audioSelector(mediaFormat)) {
                audioTrack = track
                audioFormat = mediaFormat
                Timber.d("MediaDataExtractorImpl select audio: track($track) and format is $mediaFormat.")
            } else if (videoSelector(mediaFormat)) {
                videoTrack = track
                videoFormat = mediaFormat
                Timber.d("MediaDataExtractorImpl select audio: track($track) and format is $mediaFormat.")
            }
        }
    }

    private fun handleSeek() {
        if (seekTime != -1L) {
            mediaExtractor?.run {

                audioPacketQueue.clear()
                videoPacketQueue.clear()

                if (audioTrack != -1) {
                    selectTrack(audioTrack)
                    seekTo(seekTime, MediaExtractor.SEEK_TO_PREVIOUS_SYNC)
                    advance()
                }
                if (videoTrack != -1) {
                    selectTrack(videoTrack)
                    seekTo(seekTime, MediaExtractor.SEEK_TO_PREVIOUS_SYNC)
                    advance()
                }
            }
            seekTime = -1L
        }
    }

    override fun getAudioPacket(): Packet? {
        if (audioReachEnd && audioPacketQueue.isEmpty()) {
            return null
        }
        return audioPacketQueue.take()
    }

    override fun getVideoPacket(): Packet? {
        if (videoReachEnd && videoPacketQueue.isEmpty()) {
            return null
        }
        return videoPacketQueue.take()
    }

    private fun release() {
        mediaExtractor?.release()
        mediaExtractor = null
        audioReachEnd = false
        videoReachEnd = false
        audioTrack = -1
        videoTrack = -1
        audioFormat = null
        videoFormat = null
        audioPacketQueue.clear()
        videoPacketQueue.clear()
        started.set(false)
    }

    override fun seek(position: Long) {
        seekTime = position
    }

    override fun stop() {
        started.compareAndSet(true, false)
    }

}