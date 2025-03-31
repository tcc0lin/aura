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