#include <jni.h>
#include <string>
#include "detectors/detectors.h"

jboolean frida_detect1(JNIEnv *env, jobject instance) {
    return frida_detect_by_1();
}

jboolean frida_detect2(JNIEnv *env, jobject instance) {
    return frida_detect_by_2();
}

static JNINativeMethod gMethods[] = {
        {"fridaDetect1", "()Z", (void *) frida_detect1},
        {"fridaDetect2", "()Z", (void *) frida_detect2},
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    if ((*vm).GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jclass cls = env->FindClass("com/tcc0lin/aura/detectors/FridaDetector");
    if (cls == NULL) {
        return JNI_ERR;
    }

    if (env->RegisterNatives(cls, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) < 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

