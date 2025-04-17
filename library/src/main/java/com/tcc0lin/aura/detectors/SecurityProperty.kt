package com.tcc0lin.aura.detectors

import android.content.Context
import com.tcc0lin.aura.Detail
import com.tcc0lin.aura.IDetector

class SecurityProperty(context: Context) : IDetector(context) {
    override val name = "Security Property"
    override fun run(packages: Collection<String>?, detail: Detail?): Result {
        var result = Result.NOT_FOUND
        val add: (Pair<String, Result>) -> Unit = {
            result = result.coerceAtLeast(it.second)
            detail?.add(it)
        }
        var propertyMap = DetectorManager.getSecurityProperty();
        for ((key, value) in propertyMap) {
            var model_result = "$key -> $value";
            add(model_result to if (value == "error") Result.SUSPICIOUS else Result.NOT_FOUND);
        }
        return result
    }
}