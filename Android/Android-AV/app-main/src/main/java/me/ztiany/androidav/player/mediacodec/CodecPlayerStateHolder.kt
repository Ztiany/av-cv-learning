package me.ztiany.androidav.player.mediacodec

import java.util.concurrent.CopyOnWriteArrayList
import java.util.concurrent.atomic.AtomicInteger

private const val NONE = 1
private const val STARTED = 2
private const val PAUSED = 3
private const val STOPPED = 4

typealias StateListener = (Int) -> Unit

class CodecPlayerStateHolder {

    private val onStateListeners = CopyOnWriteArrayList<StateListener>()

    private val currentState = AtomicInteger(NONE)

    fun switchToStarted(): Boolean {
        if (currentState.get() == STARTED) {
            return false
        }
        currentState.set(STARTED)
        notifyNewState()
        return true
    }

    private fun notifyNewState() {
        val newState = currentState.get()
        onStateListeners.forEach { it(newState) }
    }

}