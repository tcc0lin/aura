package com.tcc0lin.aura

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log

class RemoteService : Service() {
    companion object {
        const val TAG: String = "[Aura Detector]"
    }

    override fun onBind(intent: Intent): IBinder? {
        Log.i(TAG, "RemoteService");
        return ServiceStub()
    }

    class ServiceStub : IRemoteService.Stub() {
        override fun getsu(): Int {
            return 31221231;
        }
    }
}