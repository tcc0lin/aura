#ifndef AURA_FILE_UTILS_H
#define AURA_FILE_UTILS_H


#include <sys/types.h>
#include "../custom_libc.h"
#include "../custom_syscall.h"

#define MAX_LENGTH 256

ssize_t read_one_line(int fd, char *buf, unsigned int max_len);

int security_openat(int __dir_fd, const char *__path, int __flags, int __mode);

#endif //AURA_FILE_UTILS_H
