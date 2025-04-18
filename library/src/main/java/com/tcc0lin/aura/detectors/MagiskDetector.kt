package com.tcc0lin.aura.detectors

import android.content.Context
import com.tcc0lin.aura.Detail
import com.tcc0lin.aura.IDetector

class MagiskDetector(context: Context) : IDetector(context) {
    override val name = "Magisk Environment"
    override fun run(packages: Collection<String>?, detail: Detail?): Result {
        var result = Result.NOT_FOUND
        val add: (Pair<String, Result>) -> Unit = {
            result = result.coerceAtLeast(it.second)
            detail?.add(it)
        }
        var resultArray = DetectorManager.getDetectResult();
        for ((key, value) in DetectorManager.fridaMap) {
            add(key to Result.fromCode(resultArray[value]))
        }
        return result
    }
}