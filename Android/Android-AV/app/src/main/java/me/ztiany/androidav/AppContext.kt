package me.ztiany.androidav

import android.app.Application
import kotlin.properties.Delegates

class AppContext : Application() {

    override fun onCreate() {
        super.onCreate()
        appContext = this
    }

    companion object {

        private var appContext by Delegates.notNull<AppContext>()

        fun get() = appContext

    }

}