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
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <pthread.h>

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


#define SOCKET_PATH "\0anti_ptrace_socket"
static int socket_fd;
static int client_fd;
static pid_t child_pid;

void child_monitor(pid_t pid) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    memcpy(addr.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH));
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        LOGI("child connect failed");
        _exit(1);
    }
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("ptrace attach failed");
        return;
    }
    int status;
    waitpid(pid, &status, 0);
    while (1) {
        if (ptrace(PTRACE_CONT, pid, NULL, NULL) == -1) {
            perror("ptrace cont failed");
            break;
        }
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            break;
        }
    }
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

void *heartbeat_thread(void *) {
    while (1) {
        sleep(3);
        if (write(client_fd, "PING", 4) != 4) {
            LOGD("child process is kill, stop main process");
            kill(getpid(), SIGKILL);
        }
//        TODO: timeout logic
    }
    return NULL;
}

void make_socket() {
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    memcpy(addr.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH));
    bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr));
    listen(socket_fd, 1);
}

//Upon loading the library, this function annotated as constructor starts executing
__attribute__((constructor))
void init_for_frida_detect() {
//    pthread_create_t p_pthread_create = (pthread_create_t) dlsym(RTLD_DEFAULT, "pthread_create");
////    anti ptrace logic
//    make_socket();
//    child_pid = fork();
//    if (child_pid == 0) {
//        child_monitor(getppid());
//        _exit(0);
//    }
//    client_fd = accept(socket_fd, NULL, NULL);
//    pthread_t tid;
//    if (!p_pthread_create) {
//        fprintf(stderr, "dlsym error: %s\n", dlerror());
//    } else {
//        pthread_create(&tid, NULL, heartbeat_thread, NULL);
//    }
////    frida detect logic
//    prepare_collect_checksum();
//    pthread_t t;
//    if (!p_pthread_create) {
//        fprintf(stderr, "dlsym error: %s\n", dlerror());
//    } else {
//        p_pthread_create(&t, NULL, detect_frida_loop, NULL);
//    }
    magisk_detect_by_exec_path();
    magisk_detect_by_mountinfo();
}


jintArray get_detect_result_to_java(JNIEnv *env, jobject jobj) {
    std::vector<jint> results(g_detectionResults, g_detectionResults + g_detectionSize);
    jintArray jArray = env->NewIntArray(results.size());
    if (jArray) {
        env->SetIntArrayRegion(jArray, 0, results.size(), results.data());
    }
    LOGI("get detect result");
    return jArray;
}

jobject get_security_property_map(JNIEnv *env, jobject jobj) {
    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jmethodID hashMapInit = env->GetMethodID(hashMapClass, "<init>", "()V");
    jmethodID putMethod = env->GetMethodID(hashMapClass, "put",
                                           "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jobject hashMap = env->NewObject(hashMapClass, hashMapInit);
    init_properties();
    std::map<std::string, std::string> data;
    std::vector<const char *> keys = {
            "ro.product.model",
            "ro.product.build.fingerprint",
            "ro.product.build.version.incremental"
    };
    for (auto it = keys.begin(); it != keys.end(); it++) {
        data.insert(std::make_pair(*it, get_security_property(*it)));
    }
    free_properties();
    for (const auto &entry: data) {
        jstring key = env->NewStringUTF(entry.first.c_str());
        jstring value = env->NewStringUTF(entry.second.c_str());
        env->CallObjectMethod(hashMap, putMethod, key, value);
        env->DeleteLocalRef(key);
        env->DeleteLocalRef(value);
    }
    return hashMap;
};

static JNINativeMethod gMethods[] = {
        {"getDetectResult",     "()[I",              (void *) get_detect_result_to_java},
        {"getSecurityProperty", "()Ljava/util/Map;", (void *) get_security_property_map},
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    if ((*vm).GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jclass cls = env->FindClass("com/tcc0lin/aura/detectors/DetectorManager");
    if (cls == NULL) {
        return JNI_ERR;
    }
    if (env->RegisterNatives(cls, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) < 0) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}

