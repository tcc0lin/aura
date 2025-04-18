package com.tcc0lin.aura.detectors

import android.content.Context
import com.tcc0lin.aura.Detail
import com.tcc0lin.aura.IDetector

class AppInfo(context: Context) : IDetector(context) {
    override val name = "Security App Info"
    override fun run(packages: Collection<String>?, detail: Detail?): Result {
        var result = Result.NOT_FOUND
        val add: (Pair<String, Result>) -> Unit = {
            result = result.coerceAtLeast(it.second)
            detail?.add(it)
        }
        add("sss" to Result.FOUND);
        return result
    }
}