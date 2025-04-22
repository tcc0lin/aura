#include "detectors.h"
#include "string"
#include "../utils/logging.h"
#include "../utils/file_utils.h"
#include "../custom_syscall.h"

__attribute__((always_inline))
void magisk_detect_by_exec_path() {
    LOGI("--------------------magisk_detect_by_exec_path start--------------------");
    int status = 0;
    char *path = getenv("PATH");
    char *p = strtok(path, ":");
    char supath[PATH_MAX];
    do {
        sprintf(supath, "%s/su", p);
        if (access(supath, F_OK) == 0) {
            LOGI("Found su at %s", supath);
            status = 3;
        }
    } while ((p = strtok(NULL, ":")) != NULL);
    set_detection_value(7, status);
    LOGI("--------------------magisk_detect_by_exec_path end--------------------");
}

int scan_maps(dev_t data_dev) {
    int module = 0;
    char line[PATH_MAX];
    char maps[] = "/proc/self/maps";
    int fd = my_openat(AT_FDCWD, maps, O_RDONLY | O_CLOEXEC, 0);
    if (fd < 0) {
        LOGE("cannot open %s", maps);
        return -1;
    }
    FILE *fp = fdopen(fd, "r");
    if (fp == NULL) {
        LOGE("cannot open %s", maps);
        close(fd);
        return -1;
    }
    while (fgets(line, PATH_MAX - 1, fp) != NULL) {
        if (strchr(line, '/') == NULL) continue;
        if (strstr(line, " /system/") != NULL ||
            strstr(line, " /vendor/") != NULL ||
            strstr(line, " /product/") != NULL ||
            strstr(line, " /system_ext/") != NULL) {
            int f;
            int s;
            char p[PATH_MAX];
            sscanf(line, "%*s %*s %*s %x:%x %*s %s", &f, &s, p);
            if (makedev(f, s) == data_dev) {
                LOGW("Magisk module file %x:%x %s", f, s, p);
                module++;
            }
        }
    }
    fclose(fp);
    if (module > 0) {
        return 3;
    }
    return module;
}

dev_t scan_mountinfo_for_get_datafd() {
    int major = 0;
    int minor = 0;
    char line[PATH_MAX];
    char mountinfo[] = "/proc/self/mountinfo";
    int fd = my_openat(AT_FDCWD, mountinfo, O_RDONLY | O_CLOEXEC, 0);
    if (fd < 0) {
        LOGE("cannot open %s", mountinfo);
        return 2;
    }
    while ((read_one_line(fd, line, MAX_LINE)) > 0) {
        if (my_strstr(line, "/ /data ") != NULL) {
            sscanf(line, "%*d %*d %d:%d", &major, &minor);
            sscanf(line, "%*d %*d %d:%d", &major, &minor);
            LOGI("line: %s", line);
        }
    }
    my_close(fd);
    return makedev(major, minor);
}

int scan_mountinfo_for_device_block() {
    int mount_id, parent_id, major, minor;
    char mount_root[256], mount_point[256], mount_opts[256], fs_type[256], device_source[256], super_opts[256];
    char line[PATH_MAX];
    char mountinfo[] = "/proc/self/mountinfo";
    int fd = my_openat(AT_FDCWD, mountinfo, O_RDONLY | O_CLOEXEC, 0);
    if (fd < 0) {
        LOGE("cannot open %s", mountinfo);
    }
    while ((read_one_line(fd, line, MAX_LINE)) > 0) {
        sscanf(line, "%d %d %d:%d %255s %255s %255s %*[^-] - %255s %255s %255s", &mount_id,
               &parent_id, &major, &minor,
               mount_root, mount_point, mount_opts,
               fs_type, device_source, super_opts);
//        LOGI("device_source: %s", device_source);
        if (my_strstr(line, ".magisk") != NULL) {
            return 3;
        }
    }
    my_close(fd);
    return 0;
}

void magisk_detect_by_mountinfo() {
    LOGI("--------------------magisk_detect_by_mountinfo start--------------------");
    int status = 0;
    dev_t data_dev = scan_mountinfo_for_get_datafd();
    if (data_dev != 0) {
        status = scan_maps(data_dev);
    }
    if (status == 0) {
        status = scan_mountinfo_for_device_block();
    }
    set_detection_value(8, status);
    LOGI("--------------------magisk_detect_by_mountinfo end--------------------");
}
