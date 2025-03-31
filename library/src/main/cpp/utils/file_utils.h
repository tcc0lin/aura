#ifndef AURA_FILE_UTILS_H
#define AURA_FILE_UTILS_H


#include <sys/types.h>
#include "../custom_libc.h"
#include "../custom_syscall.h"

ssize_t read_one_line(int fd, char *buf, unsigned int max_len);

#endif //AURA_FILE_UTILS_H
