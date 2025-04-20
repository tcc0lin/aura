package com.tcc0lin.aura

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log
import com.tcc0lin.aura.detectors.DetectorManager

class RemoteService : Service() {
    companion object {
        const val TAG: String = "[Aura Detector]"

        init {
            System.loadLibrary("resplendent")
        }
    }

    override fun onBind(intent: Intent): IBinder? {
        Log.i(TAG, "RemoteService");
        return ServiceStub()
    }

    class ServiceStub : IRemoteService.Stub() {
        override fun getDetectResult(): IntArray {
            return DetectorManager.getDetectResult()
        }
    }
}