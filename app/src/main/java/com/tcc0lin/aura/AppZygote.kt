package com.tcc0lin.aura

import android.app.ZygotePreload
import android.content.pm.ApplicationInfo
import android.os.Build
import androidx.annotation.RequiresApi

@RequiresApi(Build.VERSION_CODES.Q)
class AppZygote : ZygotePreload {
    override fun doPreload(p0: ApplicationInfo) {
        TODO("Not yet implemented")
    }
}