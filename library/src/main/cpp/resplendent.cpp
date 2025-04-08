#include <jni.h>
#include <string>
#include <linux/fcntl.h>
#include <asm/unistd.h>
#include <dirent.h>
#include <linux/stat.h>
#include <sys/stat.h>
#include <linux/elf.h>
#include <dlfcn.h>
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
        frida_detect_by_solist();
        my_nanosleep(&timereq, NULL);
        // test
    }
}

typedef int (*pthread_create_t)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);


//Upon loading the library, this function annotated as constructor starts executing
__attribute__((constructor))
void init_for_frida_detect() {
    prepare_collect_checksum();
    pthread_t t;
    pthread_create_t p_pthread_create = (pthread_create_t) dlsym(RTLD_DEFAULT, "pthread_create");
    if (!p_pthread_create) {
        fprintf(stderr, "dlsym 失败: %s\n", dlerror());
    } else {
        p_pthread_create(&t, NULL, detect_frida_loop, NULL);
    }
}

#define GENERATE_JNI_FUNC(NAME) \
    extern "C" JNIEXPORT jboolean JNICALL \
    JNI_##NAME(JNIEnv* env, jobject instance) { \
        return static_cast<jboolean>(get_detect_result(#NAME)); \
    }

GENERATE_JNI_FUNC(frida_detect_by_namedpipe)
GENERATE_JNI_FUNC(frida_detect_by_threads)
GENERATE_JNI_FUNC(frida_detect_by_memdiskcompare)
GENERATE_JNI_FUNC(frida_detect_by_socket)
GENERATE_JNI_FUNC(frida_detect_by_agent)
GENERATE_JNI_FUNC(frida_detect_by_memoryscan)
GENERATE_JNI_FUNC(frida_detect_by_solist)

static JNINativeMethod gMethods[] = {
        {"fridaDetectByNamedpipe",      "()Z", (void *) JNI_frida_detect_by_namedpipe},
        {"fridaDetectByThreads",        "()Z", (void *) JNI_frida_detect_by_threads},
        {"fridaDetectByMemdiskcompare", "()Z", (void *) JNI_frida_detect_by_memdiskcompare},
        {"fridaDetectBySocket",         "()Z", (void *) JNI_frida_detect_by_socket},
        {"fridaDetectByAgent",          "()Z", (void *) JNI_frida_detect_by_agent},
        {"fridaDetectByMemoryscan",     "()Z", (void *) JNI_frida_detect_by_memoryscan},
        {"fridaDetectBySoList",     "()Z", (void *) JNI_frida_detect_by_solist},
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

