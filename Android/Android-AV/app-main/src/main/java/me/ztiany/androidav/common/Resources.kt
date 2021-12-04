@file:JvmName("Resources")

package me.ztiany.androidav.common

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.annotation.DrawableRes
import androidx.annotation.IdRes
import me.ztiany.androidav.AppContext

fun loadBitmap(@DrawableRes resourceId: Int): Bitmap = BitmapFactory.decodeResource(AppContext.get().resources, resourceId)
