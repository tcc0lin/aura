package com.tcc0lin.aura.detectors

object DetectorManager {
    external fun getDetectResult(): IntArray
    external fun getSecurityProperty(): Map<String, Any?>
    val fridaMap = mapOf(
        "frida detect by namedpipe" to 0,
        "frida detect by threads" to 1,
        "frida detect by mem&disk compare" to 2,
        "frida detect by socket" to 3,
        "frida detect by agent" to 4,
        "frida detect by memory scan" to 5,
        "frida detect by solist scan" to 6,
    )
}