package me.ztiany.androidav

import android.app.Application
import android.content.Context
import me.ztiany.androidav.common.CrashHandler
import timber.log.Timber
import kotlin.properties.Delegates


class AppContext : Application() {

    override fun onCreate() {
        super.onCreate()
        appContext = this

        //加载 native
        System.loadLibrary("androidav")

        //配置调试工具
        Timber.plant(Timber.DebugTree())
        CrashHandler.register(this)
    }

    companion object {

        private var appContext by Delegates.notNull<AppContext>()

        @JvmStatic
        fun get(): Context {
            return appContext
        }

    }

}