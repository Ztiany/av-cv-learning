package me.ztiany.androidav.opengl.jwopengl.egl.core

import android.opengl.EGLContext
import android.os.Handler
import android.os.HandlerThread
import android.os.Message
import android.view.Surface
import timber.log.Timber

class EGLEnvironment(
    private val surfaceProvider: SurfaceProvider,
    private val eglAttribute: EGLAttribute
) {

    companion object {
        private const val MSG_EGL_INIT = 6
        private const val MSG_EGL_NEW_SURFACE = 2
        private const val MSG_EGL_SURFACE_REFRESH = 3
        private const val MSG_EGL_SURFACE_DESTROYED = 4
        private const val MSG_EGL_SURFACE_RELEASE = 5

        private const val MSG_RENDERER_SURFACE_CREATED = 105
        private const val MSG_RENDERER_SURFACE_CHANGED = 106
        private const val MSG_RENDERER_DRAW = 108
    }

    private val eglCore = EGLCore()

    @Volatile private var surfaceAvailable = false

    private val eglThread = HandlerThread("EGLEnvironment")
    private val eglHandler: Handler

    var renderMode = RenderMode.Continuously

    private var hasNotified = false

    var renderer: Renderer? = null
        set(value) {
            if (field == value) {
                return
            }
            hasNotified = false
            field = value
            eglHandler.sendEmptyMessage(MSG_RENDERER_SURFACE_CREATED)
        }

    init {
        eglThread.start()
        eglHandler = Handler(eglThread.looper, ::handleMessage)
        eglHandler.sendEmptyMessage(MSG_EGL_INIT)
        surfaceProvider.start(newSurfaceProviderCallback())
    }

    private fun newSurfaceProviderCallback() = object : SurfaceProviderCallback {
        override fun onSurfaceAvailable(surface: Surface) {
            surfaceAvailable = true
            eglHandler.sendMessage(Message.obtain().apply {
                what = MSG_EGL_NEW_SURFACE
                obj = surface
            })
        }

        override fun onSurfaceChanged(surface: Surface, width: Int, height: Int) {
            eglHandler.sendMessage(Message.obtain().apply {
                what = MSG_EGL_SURFACE_REFRESH
                arg1 = width
                arg2 = height
            })
        }

        override fun onSurfaceDestroyed() {
            surfaceAvailable = false
            eglHandler.removeCallbacksAndMessages(null)
            eglHandler.sendEmptyMessage(MSG_EGL_SURFACE_DESTROYED)
        }

    }

    fun release() {
        surfaceProvider.stop()
        eglHandler.removeCallbacksAndMessages(null)
        eglHandler.sendEmptyMessage(MSG_EGL_SURFACE_RELEASE)
        eglThread.quitSafely()
    }

    private fun handleMessage(message: Message): Boolean {
        if (message.what <= 100) {
            handleEGLMessage(message)
        } else {
            handleRendererMessage(message)
        }
        return true
    }

    private fun handleEGLMessage(message: Message) {
        when (message.what) {
            MSG_EGL_INIT -> {
                Timber.d("handleEGLMessage MSG_EGL_INIT")
                eglCore.makeEglContext(message.obj as? EGLContext)
            }
            MSG_EGL_NEW_SURFACE -> {
                eglCore.makeEglWindowSurface(message.obj as Surface)
                eglCore.makeCurrent()
                eglHandler.sendEmptyMessage(MSG_RENDERER_SURFACE_CREATED)
            }
            MSG_EGL_SURFACE_REFRESH -> {
                //egl doesn't need to do anything.
                //...
                //notify renderer the surface size.
                eglHandler.sendMessage(Message.obtain().apply {
                    what = MSG_RENDERER_SURFACE_CHANGED
                    arg1 = message.arg1
                    arg2 = message.arg2
                })
                if (renderMode == RenderMode.Continuously) {
                    eglHandler.sendEmptyMessage(MSG_RENDERER_DRAW)
                }
            }
            MSG_EGL_SURFACE_DESTROYED -> {
                renderer?.onSurfaceDestroy()
                eglCore.destroySurface()
            }
            MSG_EGL_SURFACE_RELEASE -> {
                eglCore.release()
            }
        }
    }

    private fun handleRendererMessage(message: Message) {
        when (message.what) {
            MSG_RENDERER_SURFACE_CREATED -> {
                if (eglCore.isActive() && surfaceAvailable) {
                    if (!hasNotified) {
                        renderer?.onSurfaceCreated()
                        hasNotified = true
                    }
                }
            }
            MSG_RENDERER_SURFACE_CHANGED -> {
                if (eglCore.isActive() && surfaceAvailable) {
                    renderer?.onSurfaceChanged(message.arg1, message.arg2)
                }
            }
            MSG_RENDERER_DRAW -> {
                if (eglCore.isActive() && surfaceAvailable) {
                    renderer?.onDrawFrame()
                    eglCore.swapBuffers()
                }
                checkIfDrawContinuously()
            }
        }
    }

    private fun checkIfDrawContinuously() {
        if (renderMode == RenderMode.Continuously) {
            eglHandler.sendEmptyMessageDelayed(MSG_RENDERER_DRAW, 16)
        }
    }

    fun requestRender() {
        if (surfaceAvailable) {
            eglHandler.sendEmptyMessage(MSG_RENDERER_DRAW)
        }
    }

}