#include <malloc.h>
#include <linux/fcntl.h>
#include "file_utils.h"
#include "logging.h"

ssize_t read_one_line(int fd, char *buf, unsigned int max_len) {
    char b;
    ssize_t ret;
    ssize_t bytes_read = 0;
    my_memset(buf, 0, max_len);
    do {
        ret = my_read(fd, &b, 1);
        if (ret != 1) {
            if (bytes_read == 0) {
                // error or EOF
                return -1;
            } else {
                return bytes_read;
            }
        }
        if (b == '\n') {
            return bytes_read;
        }
        *(buf++) = b;
        bytes_read += 1;
    } while (bytes_read < max_len - 1);
    return bytes_read;
}

__attribute__((always_inline))
bool check_path_readlink_status(const char *originPath, int fd) {
    char readlinkPath[MAX_LENGTH] = "";
    char filePath[MAX_LENGTH] = "";
    snprintf(filePath, sizeof(filePath), "/proc/self/fd/%d", fd);
    ssize_t size = my_readlinkat(AT_FDCWD, filePath, readlinkPath, MAX_LENGTH);
    LOGI("check_path_readlink_status originPath: %s readlinkPath: %s size: %d", originPath,
         readlinkPath, size);
    if (my_strcmp(originPath, readlinkPath) != 0 && size == my_strlen(originPath)) {
        return true;
    }
    return false;
}

int security_openat(int __dir_fd, const char *__path, int __flags, int __mode) {
    int fd = 0;
    if ((fd = my_openat(AT_FDCWD, __path, O_RDONLY | O_CLOEXEC, 0)) != 0) {
        if (check_path_readlink_status(__path, fd)) {
            LOGI("Error check_path_readlink_status");
            return 0;
        }
        return fd;
    } else {
        LOGE("Error security_openat");
    }
    return 0;
}