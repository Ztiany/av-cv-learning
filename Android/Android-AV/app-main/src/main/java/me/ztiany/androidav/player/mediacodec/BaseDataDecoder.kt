package me.ztiany.androidav.player.mediacodec

import android.media.MediaCodec
import android.media.MediaFormat
import timber.log.Timber
import java.nio.ByteBuffer
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.locks.Condition
import java.util.concurrent.locks.Lock
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.thread

abstract class BaseDataDecoder(
    private val stateHolder: CodecPlayerStateHolder,
    private val mediaFormat: MediaFormat,
    private val provider: MediaDataProvider,
    private val renderer: MediaDataRenderer
) : MediaDataDecoder {

    protected lateinit var decoder: MediaCodec

    private val isRunning = AtomicBoolean(false)
    private val inputDone = AtomicBoolean(false)
    private val outputDone = AtomicBoolean(false)

    private val inputLock by lazy { ReentrantLock() }
    private val inputCondition by lazy { inputLock.newCondition() }

    private val outputLock by lazy { ReentrantLock() }
    private val outputCondition by lazy { outputLock.newCondition() }

    private val outputBufferInfo by lazy {
        MediaCodec.BufferInfo()
    }

    private val packInfo = PacketInfo(0, 0)

    override fun start() {
        if (isRunning.compareAndSet(false, true) && initDecoder(mediaFormat, renderer)) {
            decoder.start()
            startInputWorker()
            startOutputWorker()
        }
    }

    private fun startInputWorker() {
        thread {
            startWork(inputLock, inputCondition, inputDone) {
                pushData()
            }
        }
    }

    private fun startOutputWorker() {
        thread {
            startWork(outputLock, outputCondition, outputDone) {
                pullData()
            }
        }
    }

    abstract fun initDecoder(mediaFormat: MediaFormat, renderer: MediaDataRenderer): Boolean

    override fun stop() {
        isRunning.set(false)
    }

    private fun startWork(lock: Lock, condition: Condition, workDone: AtomicBoolean, task: Runnable) {
        outputDone.set(false)

        val onStateChanged: StateListener = {
            tryNotify(lock, condition)
        }
        stateHolder.addStateChanged(onStateChanged)

        while (!workDone.get()) {
            when {
                !isRunning.get() || stateHolder.isStopped -> {
                    workDone.set(true)
                    stop()
                    break
                }
                stateHolder.isPaused -> handleOnPause(lock, condition)
                stateHolder.isStarted -> {
                    task.run()
                }
            }
        }

        stateHolder.removeStateChanged(onStateChanged)
        releaseDecoder()
    }

    private fun releaseDecoder() {
        if (inputDone.get() && outputDone.get()) {
            Timber.d("releaseDecoder")
            decoder.stop()
            decoder.release()
        }
    }

    private fun handleOnPause(lock: Lock, condition: Condition) {
        try {
            lock.lock()
            while (stateHolder.isPaused) {
                Timber.d("Decoder State: isPaused = true, do await")
                condition.await(5, TimeUnit.SECONDS)
            }
        } finally {
            lock.unlock()
        }
    }

    private fun tryNotify(lock: Lock, condition: Condition) {
        try {
            lock.lock()
            if (!stateHolder.isPaused) {
                Timber.d("Decoder State: isPaused = false, do signal")
                condition.signal()
            }
        } finally {
            lock.unlock()
        }
    }

    private fun pushData() {
        if (inputDone.get()) {
            Timber.d("Decoder pushData reachEndOfStream")
            return
        }

        val inputBufferIndex = try {
            decoder.dequeueInputBuffer(10000)
        } catch (e: Exception) {
            Timber.e(e, "Decoder dequeueInputBuffer")
            -1
        }

        if (inputBufferIndex < 0) {
            //Timber.d("Decoder pushData inputBufferIndex < 0")
            return
        }

        val byteBuffer = decoder.getInputBuffer(inputBufferIndex) ?: return
        val readSize = provider.readPacket(byteBuffer, packInfo)
        Timber.d("readPacket readSize = $readSize sampleTime = ${packInfo.sampleTime}")

        if (readSize < 0) {
            decoder.queueInputBuffer(inputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM)
            inputDone.set(true)
            Timber.d("Decoder pushData reach EOF")
        } else {
            decoder.queueInputBuffer(inputBufferIndex, 0, readSize, packInfo.sampleTime, 0)
        }
    }

    private fun pullData() {
        val index = try {
            decoder.dequeueOutputBuffer(outputBufferInfo, 10000)
        } catch (e: Exception) {
            Timber.e(e, "Decoder dequeueOutputBuffer")
            -1
        }

        when {
            index == MediaCodec.INFO_TRY_AGAIN_LATER -> {
                Timber.d("Decoder INFO_TRY_AGAIN_LATER")
                return
            }
            index == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED -> {
                Timber.d("Decoder INFO_OUTPUT_BUFFERS_CHANGED")
            }
            index == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED -> {
                val outputFormat = decoder.outputFormat
                //It's not necessary to update format.
                //renderer.updateMediaFormat(outputFormat)
                Timber.d("Decoder INFO_OUTPUT_FORMAT_CHANGED with $outputFormat")
            }
            outputBufferInfo.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0 -> {
                Timber.d("Decoder pullData reach EOS")
                outputDone.set(true)
            }
            index >= 0 -> {
                decoder.getOutputBuffer(index)?.let {
                    deliverFrame(renderer, it, outputBufferInfo, index)
                }
            }
        }
    }

    abstract fun deliverFrame(renderer: MediaDataRenderer, data: ByteBuffer, outputBufferInfo: MediaCodec.BufferInfo, index: Int)

}