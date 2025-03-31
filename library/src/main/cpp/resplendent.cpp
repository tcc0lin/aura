#include <jni.h>
#include <string>
#include <linux/fcntl.h>
#include <asm/unistd.h>
#include <dirent.h>
#include <linux/stat.h>
#include <sys/stat.h>
#include <linux/elf.h>
#include "detectors/detectors.h"
#include "custom_syscall.h"
#include "custom_libc.h"
#include "utils/logging.h"
#include "utils/file_utils.h"


static inline void *detect_frida_loop(void *pargs) {
    struct timespec timereq;
    timereq.tv_sec = 5;
    timereq.tv_nsec = 0;
    while (1) {
        LOGI("detect_frida_loop");
        frida_detect_by_threads();
        frida_detect_by_namedpipe();
        frida_detect_by_memdiskcompare();
        frida_detect_by_socket();
        frida_detect_by_agent();
        frida_detect_by_memoryscan();
        my_nanosleep(&timereq, NULL);
        // test
    }
}


//Upon loading the library, this function annotated as constructor starts executing
__attribute__((constructor))
void init_for_frida_detect() {
    prepare_collect_checksum();
    pthread_t t;
    pthread_create(&t, NULL, detect_frida_loop, NULL);
}

jboolean frida_detect_by_namedpipe_wrapper(JNIEnv *env, jobject instance) {
    return frida_detect_by_namedpipe_result();
}

jboolean frida_detect_by_threads_wrapper(JNIEnv *env, jobject instance) {
    return frida_detect_by_threads_result();
}

jboolean frida_detect_by_memdiskcompare_wrapper(JNIEnv *env, jobject instance) {
    return frida_detect_by_memdiskcompare_result();
}

jboolean frida_detect_by_socket_wrapper(JNIEnv *env, jobject instance) {
    return frida_detect_by_socket_result();
}

jboolean frida_detect_by_agent_wrapper(JNIEnv *env, jobject instance) {
    return frida_detect_by_agent_result();
}

jboolean frida_detect_by_memoryscan_wrapper(JNIEnv *env, jobject instance) {
    return frida_detect_by_memoryscan_result();
}

static JNINativeMethod gMethods[] = {
        {"fridaDetectByNamedpipe",      "()Z", (void *) frida_detect_by_namedpipe_wrapper},
        {"fridaDetectByThreads",        "()Z", (void *) frida_detect_by_threads_wrapper},
        {"fridaDetectByMemdiskcompare", "()Z", (void *) frida_detect_by_memdiskcompare_wrapper},
        {"fridaDetectBySocket",         "()Z", (void *) frida_detect_by_socket_wrapper},
        {"fridaDetectByAgent",          "()Z", (void *) frida_detect_by_agent_wrapper},
        {"fridaDetectByMemoryscan",     "()Z", (void *) frida_detect_by_memoryscan_wrapper},
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

