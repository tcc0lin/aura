package com.tcc0lin.aura

import android.annotation.SuppressLint
import android.app.Application
import android.content.Context

class MyApplication : Application() {

    companion object {
        @SuppressLint("StaticFieldLeak")
        lateinit var appContext: Context
    }

    init {
        System.loadLibrary("resplendent")
    }

    override fun onCreate() {
        super.onCreate()
        appContext = this
    }
}
