package me.ztiany.androidav.player.mediacodec

import android.media.MediaCodec
import android.media.MediaCodecList
import android.media.MediaFormat
import timber.log.Timber
import java.nio.ByteBuffer
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.thread

class AudioDataDecoder(
    private val mediaFormat: MediaFormat,
    private val stateHolder: CodecPlayerStateHolder,
    private val provider: MediaDataProvider,
    private val renderer: MediaDataRenderer
) : MediaDataDecoder {

    private lateinit var decoder: MediaCodec

    private val isRunning = AtomicBoolean(false)

    private var audioInputDone = false
    private var audioOutputDone = false

    private val lock by lazy { ReentrantLock() }
    private val condition by lazy { lock.newCondition() }

    private val outputBufferInfo by lazy {
        MediaCodec.BufferInfo()
    }

    private var unConsumedPaket: PacketInfo? = null

    override fun start() {
        if (isRunning.compareAndSet(false, true) && initAudioDecoder()) {
            thread {
                audioInputDone = false
                audioInputDone = false
                startWork()
            }
        }
    }

    private fun initAudioDecoder(): Boolean {
        return try {
            val name = MediaCodecList(MediaCodecList.ALL_CODECS).findDecoderForFormat(mediaFormat)
            Timber.d("initAudioDecoder.findDecoderForFormat: $name")
            decoder = MediaCodec.createByCodecName(name)
            decoder.configure(mediaFormat, null, null, 0)
            decoder.start()
            Timber.d("initAudioDecoder success")
            true
        } catch (e: Exception) {
            Timber.e(e, "initAudioDecoder error")
            false
        }
    }

    override fun stop() {
        isRunning.set(false)
    }

    private fun startWork() {
        val onStateChanged: StateListener = {
            tryNotify()
        }
        stateHolder.addStateChanged(onStateChanged)

        while (isRunning.get()) {
            when {
                stateHolder.isStopped || audioOutputDone -> {
                    Timber.d("Audio State: isStopped = ${stateHolder.isStopped}")
                    Timber.d("Audio State: audioOutputDone = $audioOutputDone")
                    stop()
                    break
                }
                stateHolder.isPaused -> handleOnPause()
                stateHolder.isStarted -> onWorking()
            }
        }

        stateHolder.removeStateChanged(onStateChanged)
        releaseDecoder()
    }

    private fun onWorking() {
        pushData()
        pullData()
    }

    private fun handleOnPause() {
        try {
            lock.lock()
            while (stateHolder.isPaused) {
                Timber.d("Audio State: isPaused = ${stateHolder.isPaused}")
                condition.await(1, TimeUnit.SECONDS)
            }
        } finally {
            lock.unlock()
        }
    }

    private fun tryNotify() {
        try {
            lock.lock()
            if (!stateHolder.isPaused) {
                Timber.d("Audio State: isPaused = false, do signal")
                condition.signal()
            }
        } finally {
            lock.unlock()
        }
    }

    private fun pushData() {
        if (audioInputDone) {
            Timber.d("AudioDataDecoder pushData reachEndOfStream")
            return
        }

        val inputBufferIndex = try {
            decoder.dequeueInputBuffer(1000)
        } catch (e: Exception) {
            -1
        }

        if (inputBufferIndex < 0) {
            //Timber.d("AudioDataDecoder pushData inputBufferIndex < 0")
            return
        }

        val byteBuffer = decoder.getInputBuffer(inputBufferIndex) ?: return
        byteBuffer.clear()
        val packet = unConsumedPaket ?: provider.getPacket()
        fillData(packet, inputBufferIndex, byteBuffer)
    }

    private var times = 0

    private fun fillData(packet: PacketInfo?, inputBufferIndex: Int, byteBuffer: ByteBuffer) {
        if (packet == null || packet.size < 0) {
            decoder.queueInputBuffer(inputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM)
            audioInputDone = true
            Timber.d("Push BUFFER_FLAG_END_OF_STREAM")
        } else {
            val inRemaining = byteBuffer.remaining()
            val outRemaining = packet.data.remaining()
            Timber.d("${times++}, inRemaining = $inRemaining outRemaining = $outRemaining inCapacity = ${byteBuffer.capacity()} outCapacity = ${packet.data.capacity()}")

            if (inRemaining == outRemaining) {
                Timber.d("inRemaining == outRemaining")

                unConsumedPaket = null
                byteBuffer.put(packet.data)
                decoder.queueInputBuffer(inputBufferIndex, 0, inRemaining, packet.sampleTime, 0)

            } else if (inRemaining < outRemaining) {
                Timber.d("inRemaining < outRemaining")

                val duplicate = packet.data.duplicate()
                duplicate.limit(duplicate.position() + inRemaining)
                byteBuffer.put(duplicate)
                decoder.queueInputBuffer(inputBufferIndex, 0, inRemaining, packet.sampleTime, 0)

                packet.data.position(packet.data.position() + inRemaining)
                unConsumedPaket = packet

            } else/*inRemaining > outRemaining*/ {
                Timber.d("inRemaining > outRemaining")

                unConsumedPaket = null
                byteBuffer.put(packet.data)
                fillData(provider.getPacket(), inputBufferIndex, byteBuffer)
            }

        }
    }

    private fun pullData() {
        val index = try {
            decoder.dequeueOutputBuffer(outputBufferInfo, 1000)
        } catch (e: Exception) {
            -1
        }

        when {
            index == MediaCodec.INFO_TRY_AGAIN_LATER -> {
                //Timber.d("INFO_TRY_AGAIN_LATER")
                return
            }
            index == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED -> {
                Timber.d("INFO_OUTPUT_BUFFERS_CHANGED")
            }
            index == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED -> {
                val outputFormat = decoder.outputFormat
                Timber.d("INFO_OUTPUT_FORMAT_CHANGED with $outputFormat")
            }
            index == MediaCodec.BUFFER_FLAG_END_OF_STREAM -> {
                Timber.d("Pull BUFFER_FLAG_END_OF_STREAM")
                audioOutputDone = true
            }
            index >= 0 -> {
                decoder.getOutputBuffer(index)?.let {
                    renderer.render(it, outputBufferInfo)
                    decoder.releaseOutputBuffer(index, false)
                }
            }
        }
    }

    private fun releaseDecoder() {
        decoder.stop()
        decoder.release()
    }

}