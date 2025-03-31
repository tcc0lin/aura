package com.tcc0lin.aura.detectors

import android.content.Context
import com.tcc0lin.aura.Detail
import com.tcc0lin.aura.IDetector

class FridaDetector(context: Context) : IDetector(context) {
    override val name = "Frida hooks"
    external fun fridaDetectByNamedpipe(): Boolean
    external fun fridaDetectByThreads(): Boolean
    external fun fridaDetectByMemdiskcompare(): Boolean
    external fun fridaDetectBySocket(): Boolean
    external fun fridaDetectByAgent(): Boolean
    external fun fridaDetectByMemoryscan(): Boolean
    override fun run(packages: Collection<String>?, detail: Detail?): Result {
        var result = Result.NOT_FOUND
        val add: (Pair<String, Result>) -> Unit = {
            result = result.coerceAtLeast(it.second)
            detail?.add(it)
        }
        add("frida detect by namedpipe" to if (fridaDetectByNamedpipe()) Result.FOUND else Result.NOT_FOUND)
        add("frida detect by threads" to if (fridaDetectByThreads()) Result.FOUND else Result.NOT_FOUND)
        add("frida detect by mem&disk compare" to if (fridaDetectByMemdiskcompare()) Result.FOUND else Result.NOT_FOUND)
        add("frida detect by socket" to if (fridaDetectBySocket()) Result.FOUND else Result.NOT_FOUND)
        add("frida detect by agent" to if (fridaDetectByAgent()) Result.FOUND else Result.NOT_FOUND)
        add("frida detect by memory scan" to if (fridaDetectByMemoryscan()) Result.FOUND else Result.NOT_FOUND)
        return result
    }
}