#include <linux/elf.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <linux/fcntl.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <map>

#ifndef AURA_DETECTORS_H
#define AURA_DETECTORS_H

#endif //AURA_DETECTORS_H

#define MAX_LINE 512
#define MAX_LENGTH 256
#define NUM_LIBS 1
#define LIBC "libc.so"
static const char *PROC_STATUS = "/proc/self/task/%s/status";
static const char *PROC_TASK = "/proc/self/task";
static const char *PROC_FD = "/proc/self/fd";
static const char *PROC_FD_FORMAT = "/proc/self/fd/%s";
static const char *PROC_MAPS = "/proc/self/maps";
static const char *PROC_SMAPS = "/proc/self/smaps";
static const char *TMP_DIR = "/data/local/tmp";
// static const char *libstocheck[NUM_LIBS] = {"libresplendent.so", LIBC};
static const char *libstocheck[NUM_LIBS] = {LIBC};
static const char *FRIDA_NAMEDPIPE_LINJECTOR = "linjector";
static const char *FRIDA_THREAD_GUM_JS_LOOP = "gum-js-loop";
static const char *FRIDA_THREAD_GMAIN = "gmain";
static const char *FRIDA_THREAD_GDBUS = "gdbus";
static const char *FRIDA_THREAD_POOL_FRIDA = "pool-frida";
static const char *FRIDA_AGENT = "frida-agent";

#if defined(__arm__) || defined(__i386__)  // 32位架构
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
#elif defined(__aarch64__) || defined(__x86_64__)  // 64位架构
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
#else
#error "Unknown architecture!"
#endif

//Structure to hold the details of executable section of library
typedef struct stExecSection {
    int execSectionCount;
    unsigned long offset[2];
    unsigned long memsize[2];
    unsigned long checksum[2];
    unsigned long startAddrinMem;
} execSection;

//Include more libs as per your need, but beware of the performance bottleneck especially
//when the size of the libraries are > few MBs
static execSection *elfSectionArr[NUM_LIBS] = {NULL};

static std::map<char *, int> detect_result;

void prepare_collect_checksum();

// result
int get_detect_result(char *key);

// real logic
void frida_detect_by_namedpipe();

void frida_detect_by_threads();

void frida_detect_by_memdiskcompare();

void frida_detect_by_socket();

void frida_detect_by_agent();

void frida_detect_by_memoryscan();

void frida_detect_by_solist();

// TODO
// add libart.so detect: https://bbs.kanxue.com/thread-268586-1.htm
//check export func
