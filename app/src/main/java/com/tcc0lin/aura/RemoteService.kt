package com.tcc0lin.aura

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log

class RemoteService : Service() {
    companion object {
        val TAG: String = "[Aura Detector]"
    }

    override fun onBind(intent: Intent): IBinder? {
        Log.i(TAG, "RemoteService");
        return ServiceStub()
    }

    class ServiceStub : IRemoteService.Stub() {
        override fun add(a: Int, b: Int): Int {
            Log.i(RemoteService.TAG, "RemoteService add");
            return 3;
        }
    }
}