package com.tcc0lin.aura

import android.content.Context

typealias Detail = MutableCollection<Pair<String, IDetector.Result>>

abstract class IDetector(protected val context: Context) {

    enum class Result(val code: Int) {
        NOT_FOUND(0), METHOD_UNAVAILABLE(1), SUSPICIOUS(2), FOUND(3);

        companion object {
            // 根据code查找枚举值
            fun fromCode(code: Int): Result {
                return values().firstOrNull { it.code == code } ?: NOT_FOUND
            }
        }
    }

    abstract val name: String

    abstract fun run(packages: Collection<String>?, detail: Detail?): Result
}