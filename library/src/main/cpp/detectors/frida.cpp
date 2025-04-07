#include <link.h>
#include "detectors.h"
#include "../utils/logging.h"
#include "../custom_syscall.h"
#include "../custom_libc.h"
#include "../utils/file_utils.h"

__attribute__((always_inline))
void frida_detect_by_namedpipe() {
    LOGI("--------------------frida_detect_by_namedpipe start--------------------");
    DIR *dir = opendir(PROC_FD);
    if (dir != NULL) {
        struct dirent *entry = NULL;
        while ((entry = readdir(dir)) != NULL) {
            struct stat filestat;
            char buf[MAX_LENGTH] = "";
            char filePath[MAX_LENGTH] = "";
            snprintf(filePath, sizeof(filePath), PROC_FD_FORMAT, entry->d_name);
            // LOGI("filePath: %s", filePath);
            lstat(filePath, &filestat);
            if ((filestat.st_mode & S_IFMT) == S_IFLNK) {
                //TODO: Another way is to check if filepath belongs to a path not related to system or the app
                my_readlinkat(AT_FDCWD, filePath, buf, MAX_LENGTH);
                // LOGI("readlink filePath: %s realPath: %s", filePath, buf);
                if (NULL != my_strstr(buf, FRIDA_NAMEDPIPE_LINJECTOR)) {
                    detect_result.insert({"frida_detect_by_namedpipe", true});
                    LOGI("Frida specific named pipe found. Act now!!!");
                }
                if (NULL != my_strstr(buf, TMP_DIR)) {
                    detect_result.insert({"frida_detect_by_namedpipe", true});
                    LOGI("Frida specific named pipe found. Act now!!!");
                }
            }
        }
    }
    closedir(dir);
    LOGI("--------------------frida_detect_by_namedpipe end--------------------");
};

__attribute__((always_inline))
void frida_detect_by_threads() {
    LOGI("--------------------frida_detect_by_threads start--------------------");
    DIR *dir = opendir(PROC_TASK);
    if (dir != NULL) {
        struct dirent *entry = NULL;
        while ((entry = readdir(dir)) != NULL) {
            char filePath[MAX_LENGTH] = "";
            if (0 == my_strcmp(entry->d_name, ".") || 0 == my_strcmp(entry->d_name, "..")) {
                continue;
            }
            snprintf(filePath, sizeof(filePath), PROC_STATUS, entry->d_name);
            // LOGI("filePath: %s", filePath);
            int fd = security_openat(AT_FDCWD, filePath, O_RDONLY | O_CLOEXEC, 0);
            if (fd != 0) {
                char buf[MAX_LENGTH] = "";
                read_one_line(fd, buf, MAX_LENGTH);
                // LOGI("buf: %s", buf);
                if (my_strstr(buf, FRIDA_THREAD_GUM_JS_LOOP) ||
                    my_strstr(buf, FRIDA_THREAD_GMAIN) ||
                    my_strstr(buf, FRIDA_THREAD_GDBUS) ||
                    my_strstr(buf, FRIDA_THREAD_POOL_FRIDA)
                        ) {
                    //Kill the thread. This freezes the app. Check if it is an anticpated behaviour
                    //int tid = my_atoi(entry->d_name);
                    //int ret = my_tgkill(getpid(), tid, SIGSTOP);
                    detect_result.insert({"frida_detect_by_threads", true});
                    LOGI("Frida specific thread found. Act now!!!");
                }
                my_close(fd);
            }
        }
        closedir(dir);
    }
    LOGI("--------------------frida_detect_by_threads end--------------------");
}

void parse_proc_maps_to_fetch_path(char **filepaths) {
    LOGI("--------------------parse_proc_maps_to_fetch_path start--------------------");
    int fd = 0;
    char map[MAX_LINE];
    int counter = 0;
    if ((fd = my_openat(AT_FDCWD, PROC_MAPS, O_RDONLY | O_CLOEXEC, 0)) != 0) {
        while ((read_one_line(fd, map, MAX_LINE)) > 0) {
            for (int i = 0; i < NUM_LIBS; i++) {
                if (my_strstr(map, libstocheck[i]) != NULL) {
                    char tmp[MAX_LENGTH] = "";
                    char path[MAX_LENGTH] = "";
                    char buf[5] = "";
                    sscanf(map, "%s %s %s %s %s %s", tmp, buf, tmp, tmp, tmp, path);
                    LOGI("Map [%s]", map);
                    if (buf[2] == 'x') {
                        size_t size = my_strlen(path) + 1;
                        filepaths[i] = static_cast<char *>(malloc(size));
                        my_strlcpy(filepaths[i], path, size);
                        counter++;
                        LOGI("filepaths add");
                    }
                }
            }
            if (counter == NUM_LIBS)
                break;
        }
        my_close(fd);
    }
    LOGI("--------------------parse_proc_maps_to_fetch_path end--------------------");
}

unsigned long checksum(void *buffer, size_t len) {
    unsigned long seed = 0;
    uint8_t *buf = (uint8_t *) buffer;
    size_t i;
    for (i = 0; i < len; ++i)
        seed += (unsigned long) (*buf++);
    return seed;
}

__attribute__((always_inline))
bool fetch_checksum_of_library(const char *filePath, execSection **pTextSection) {
    LOGI("--------------------fetch_checksum_of_library start--------------------");
    Elf_Ehdr ehdr;
    Elf_Shdr sectHdr;
    int fd;
    int execSectionCount = 0;
    fd = security_openat(AT_FDCWD, filePath, O_RDONLY | O_CLOEXEC, 0);
    if (fd < 0) {
        return NULL;
    }
    my_read(fd, &ehdr, sizeof(Elf_Ehdr));
    my_lseek(fd, (off_t) ehdr.e_shoff, SEEK_SET);
    unsigned long memsize[2] = {0};
    unsigned long offset[2] = {0};
    for (int i = 0; i < ehdr.e_shnum; i++) {
        my_memset(&sectHdr, 0, sizeof(Elf_Shdr));
        my_read(fd, &sectHdr, sizeof(Elf_Shdr));
        LOGI("SectionHeader: [%d][%ld]", sectHdr.sh_name, sectHdr.sh_flags);
        //Typically PLT and Text Sections are executable sections which are protected
        if (sectHdr.sh_flags & SHF_EXECINSTR) {
            LOGI("SectionHeader exec: [%d][%ld]", sectHdr.sh_name, sectHdr.sh_flags);
            offset[execSectionCount] = sectHdr.sh_offset;
            memsize[execSectionCount] = sectHdr.sh_size;
            execSectionCount++;
            if (execSectionCount == 2) {
                break;
            }
        }
    }
    if (execSectionCount == 0) {
        LOGW("No executable section found. Suspicious");
        my_close(fd);
        return false;
    }
    //This memory is not released as the checksum is checked in a thread
    *pTextSection = static_cast<execSection *>(malloc(sizeof(execSection)));
    (*pTextSection)->execSectionCount = execSectionCount;
    (*pTextSection)->startAddrinMem = 0;
    for (int i = 0; i < execSectionCount; i++) {
        my_lseek(fd, offset[i], SEEK_SET);
        uint8_t *buffer = static_cast<uint8_t *>(malloc(memsize[i] * sizeof(uint8_t)));
        my_read(fd, buffer, memsize[i]);
        (*pTextSection)->offset[i] = offset[i];
        (*pTextSection)->memsize[i] = memsize[i];
        (*pTextSection)->checksum[i] = checksum(buffer, memsize[i]);
        free(buffer);
        LOGW("ExecSection: i-[%d], offset-[%ld], memsize-[%ld], checksum-[%ld]", i, offset[i],
             memsize[i],
             (*pTextSection)->checksum[i]);
    }
    my_close(fd);
    LOGI("--------------------fetch_checksum_of_library end--------------------");
    return true;
}


bool scan_executable_segments(char *map, execSection *pElfSectArr, const char *libraryName) {
    unsigned long start, end;
    char buf[MAX_LINE] = "";
    char path[MAX_LENGTH] = "";
    char tmp[100] = "";
    sscanf(map, "%lx-%lx %s %s %s %s %s", &start, &end, buf, tmp, tmp, tmp, path);
    if (buf[2] == 'x') {
        if (buf[0] == 'r') {
            uint8_t *buffer = NULL;
            buffer = (uint8_t *) start;
            for (int i = 0; i < pElfSectArr->execSectionCount; i++) {
                if (start + pElfSectArr->offset[i] + pElfSectArr->memsize[i] > end) {
                    if (pElfSectArr->startAddrinMem != 0) {
                        buffer = (uint8_t *) pElfSectArr->startAddrinMem;
                        pElfSectArr->startAddrinMem = 0;
                        break;
                    }
                }
            }
            for (int i = 0; i < pElfSectArr->execSectionCount; i++) {
                unsigned long output = checksum(buffer + pElfSectArr->offset[i],
                                                pElfSectArr->memsize[i]);
                // LOGI("i: %d,mem: %ld,disk: %ld", i, output, pElfSectArr->checksum[i]);
                if (output != pElfSectArr->checksum[i]) {
                    LOGI("Executable Section Manipulated, maybe due to Frida or other hooking framework Act Now!!!");
                    return true;
                }
            }
        } else {
            char ch[10] = "", ch1[10] = "";
            __system_property_get("ro.build.version.release", ch);
            __system_property_get("ro.system.build.version.release", ch1);
            int version = my_atoi(ch);
            int version1 = my_atoi(ch1);
            if (version < 10 || version1 < 10) {
                LOGI("Suspicious to get XOM in version < Android10");
            } else {
                if (0 == my_strncmp(libraryName, LIBC, my_strlen(LIBC))) {
                    //If it is not readable, then most likely it is not manipulated by Frida
                    LOGI("LIBC Executable Section not readable!");
                } else {
                    LOGI("Suspicious to get XOM for non-system library on Android 10 and above");
                }
            }
        }
    } else {
        if (buf[0] == 'r') {
            pElfSectArr->startAddrinMem = start;
        }
    }
    return false;
}

__attribute__((always_inline))
void frida_detect_by_memdiskcompare() {
    LOGI("--------------------frida_detect_by_memdiskcompare start--------------------");
    int fd = 0;
    char map[MAX_LINE];
    if ((fd = security_openat(AT_FDCWD, PROC_MAPS, O_RDONLY | O_CLOEXEC, 0)) != 0) {
        while ((read_one_line(fd, map, MAX_LINE)) > 0) {
            for (int i = 0; i < NUM_LIBS; i++) {
                if (my_strstr(map, libstocheck[i]) != NULL) {
                    // LOGI("detect line: %s", map);
                    if (true ==
                        scan_executable_segments(map, elfSectionArr[i], libstocheck[i])) {
                        detect_result.insert({"frida_detect_by_memdiskcompare", true});
                        break;
                    }
                }
            }
        }
    } else {
        LOGI("Error opening /proc/self/maps. That's usually a bad sign.");
    }
    my_close(fd);
    LOGI("--------------------frida_detect_by_memdiskcompare end--------------------");
}

__attribute__((always_inline))
void prepare_collect_checksum() {
    char *filePaths[NUM_LIBS];
    parse_proc_maps_to_fetch_path(filePaths);
    for (int i = 0; i < NUM_LIBS; i++) {
        fetch_checksum_of_library(filePaths[i], &elfSectionArr[i]);
        if (filePaths[i] != NULL) {
            free(filePaths[i]);
        }
    }
}

__attribute__((always_inline))
void frida_detect_by_socket() {
    LOGI("--------------------frida_detect_by_socket start--------------------");
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(sa.sin_addr));
    int sock;
    int fd;
    char map[MAX_LINE];
    char res[7];
    int num_found;
    int ret;
    int i;
    for (i = 0; i <= 65535; i++) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        sa.sin_port = htons(i);
        if (connect(sock, (struct sockaddr *) &sa, sizeof sa) != -1) {
            memset(res, 0, 7);
            send(sock, "\x00", 1, NULL);
            send(sock, "AUTH\r\n", 6, NULL);
            usleep(100); // Give it some time to answer
            if ((ret = recv(sock, res, 6, MSG_DONTWAIT)) != -1) {
                if (strcmp(res, "REJECT") == 0) {
                    LOGI("FRIDA DETECTED [1] - frida server running on port %d!");
                }
            }
        }
        close(sock);
    }
    LOGI("--------------------frida_detect_by_socket end--------------------");
}

__attribute__((always_inline))
void frida_detect_by_agent() {
    int fd = 0;
    char map[MAX_LINE];
    if ((fd = security_openat(AT_FDCWD, PROC_MAPS, O_RDONLY | O_CLOEXEC, 0)) != 0) {
        while ((read_one_line(fd, map, MAX_LINE)) > 0) {
            if (my_strstr(map, FRIDA_AGENT) != NULL) {
                LOGI("detect line: %s", map);
                detect_result.insert({"frida_detect_by_agent", true});
            }
            if (my_strstr(map, TMP_DIR) != NULL) {
                LOGI("detect line: %s", map);
                detect_result.insert({"frida_detect_by_agent", true});
            }
        }
    } else {
        LOGI("Error opening /proc/self/maps. That's usually a bad sign.");
    }
    my_close(fd);

    if ((fd = security_openat(AT_FDCWD, PROC_SMAPS, O_RDONLY | O_CLOEXEC, 0)) != 0) {
        while ((read_one_line(fd, map, MAX_LINE)) > 0) {
            if (my_strstr(map, FRIDA_AGENT) != NULL) {
                LOGI("detect line: %s", map);
                detect_result.insert({"frida_detect_by_agent", true});
            }
            if (my_strstr(map, TMP_DIR) != NULL) {
                LOGI("detect line: %s", map);
                detect_result.insert({"frida_detect_by_agent", true});
            }
        }
    } else {
        LOGI("Error opening /proc/self/maps. That's usually a bad sign.");
    }
    my_close(fd);
}

bool find_mem_string(unsigned long start, unsigned long end, char *bytes, unsigned int len) {
    char *pmem = (char *) start;
    int matched = 0;
    while ((unsigned long) pmem < (end - len)) {
        if (*pmem == bytes[0]) {
            matched = 1;
            char *p = pmem + 1;
            while (*p == bytes[matched] && (unsigned long) p < end) {
                matched++;
                p++;
            }
            if (matched >= len) {
                return true;
            }
        }
        pmem++;
    }
    return false;
}

__attribute__((always_inline))
void frida_detect_by_memoryscan() {
    static char keyword[] = "frida";
    char permission[512];
    unsigned long start, end;
    int fd = 0;
    char map[MAX_LINE];
    if ((fd = security_openat(AT_FDCWD, PROC_MAPS, O_RDONLY | O_CLOEXEC, 0)) != 0) {
        while ((read_one_line(fd, map, MAX_LINE)) > 0) {
            sscanf(map, "%lx-%lx %s", &start, &end, permission);
            // ignore base.apk
            if (my_strstr(map, "base")) {
                continue;
            }
            if (permission[2] == 'x') {
                // LOGI("line: %s", map);
                if (find_mem_string(start, end, (char *) keyword, 5)) {
                    LOGI("check");
                    detect_result.insert({"frida_detect_by_memoryscan", true});
                    break;
                }
            }
        }
        my_close(fd);
    }
}

static int callback(struct dl_phdr_info *info, size_t size, void *data) {
    if (my_strstr(info->dlpi_name, FRIDA_AGENT) != NULL) {
        LOGI("detect line: %s", info->dlpi_name);
        detect_result.insert({"frida_detect_by_solist", true});
    }
    return 0;
}

void frida_detect_by_solist() {
    dl_iterate_phdr(callback, NULL);
}

__attribute__((always_inline))
bool get_detect_result(char *key) {
    auto it = detect_result.find(key);
    if (it != detect_result.end()) {
        return it->second;
    }
    return false;
};