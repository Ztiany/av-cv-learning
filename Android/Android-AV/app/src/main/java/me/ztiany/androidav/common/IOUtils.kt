@file:JvmName("IOUtils")

package me.ztiany.androidav.common

import timber.log.Timber
import java.io.Closeable
import java.io.IOException

fun Closeable?.closeSafely() {
    if (this != null) {
        try {
            this.close()
        } catch (e: IOException) {
            Timber.e(e)
        }
    }
}

fun closeAllSafely(vararg closeables: Closeable?) {
    closeables.forEach {
        it.closeSafely()
    }
}