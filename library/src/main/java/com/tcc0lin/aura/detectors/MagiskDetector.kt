package com.tcc0lin.aura.detectors

import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.os.IBinder
import android.os.RemoteException
import android.util.Log
import com.tcc0lin.aura.Detail
import com.tcc0lin.aura.IDetector
import com.tcc0lin.aura.IRemoteService
import com.tcc0lin.aura.RemoteService
import com.tcc0lin.aura.RemoteService.Companion.TAG

class MagiskDetector(context: Context) : IDetector(context) {
    var mRemoteService: IRemoteService? = null
    override val name = "Magisk Environment"
    override fun run(packages: Collection<String>?, detail: Detail?): Result {
        if (mRemoteService == null) {
            initServiceClient(context)
        }
        var result = Result.NOT_FOUND
        val add: (Pair<String, Result>) -> Unit = {
            result = result.coerceAtLeast(it.second)
            detail?.add(it)
        }
        try {
            val sss = mRemoteService?.getsu();
            Log.i(TAG, "RemoteService call getsu $sss");
        } catch (e: RemoteException) {
            Log.i(TAG, "RemoteException: ${e.message}")
        } catch (e: Exception) {
            Log.i(TAG, "General Exception: ${e.message}")
        }
        add("result" to Result.NOT_FOUND)
        return result
    }

    fun initServiceClient(context: Context) {
        Log.i(TAG, "onServiceInit");
        var connection = object : ServiceConnection {
            override fun onServiceConnected(name: ComponentName?, service: IBinder?) {
                Log.i(TAG, "onServiceConnected")
                try {
                    mRemoteService = IRemoteService.Stub.asInterface(service)
                } catch (e: RemoteException) {
                    Log.i(TAG, "RemoteException: ${e.message}")
                } catch (e: Exception) {
                    Log.i(TAG, "General Exception: ${e.message}")
                }
            }

            override fun onServiceDisconnected(name: ComponentName?) {
                Log.i(TAG, "onServiceDisconnected")
            }
        }
        context.bindService(
            Intent(context, RemoteService::class.java), connection, Context.BIND_AUTO_CREATE
        )
    }
}