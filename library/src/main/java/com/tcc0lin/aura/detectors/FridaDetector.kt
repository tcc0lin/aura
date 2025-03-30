package com.tcc0lin.aura.detectors

import android.content.Context
import com.tcc0lin.aura.Detail
import com.tcc0lin.aura.IDetector

class FridaDetector(context: Context) : IDetector(context) {
    override val name = "Frida hooks"

    external fun fridaDetect1(): Boolean

    external fun fridaDetect2(): Boolean

    override fun run(packages: Collection<String>?, detail: Detail?): Result {
        var result = Result.NOT_FOUND
        val add: (Pair<String, Result>) -> Unit = {
            result = result.coerceAtLeast(it.second)
            detail?.add(it)
        }
        add("frida detect1" to if (fridaDetect1()) Result.FOUND else Result.NOT_FOUND)
        add("frida detect2" to if (fridaDetect2()) Result.FOUND else Result.NOT_FOUND)
        return result
    }
}