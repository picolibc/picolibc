#ifndef _HEXAGON_SEMIHOST_H_
#define _HEXAGON_SEMIHOST_H_

#include <stdint.h>
#include <sys/stat.h>

/* System call codes */
enum hexagon_system_call_code {
    SYS_OPEN = 1,
    SYS_CLOSE = 2,
    SYS_READ = 6,
    SYS_WRITE = 5,
    SYS_SEEK = 10,
    SYS_FLEN = 12,
    SYS_REMOVE = 14,
    SYS_GET_CMDLINE = 21,
    SYS_EXIT = 24,
    SYS_FTELL = 0x100,
};

/* Software interrupt */
#define SWI "trap0 (#0)"

/* Hexagon semihosting calls */
int flen(int fd);
int hexagon_ftell(int fd);
int sys_get_cmdline(char *buffer, int count);

int hexagon_semihost(enum hexagon_system_call_code code, int *args);

void hexagon_semihost_errno(int err);
enum {
    HEX_EPERM = 1,
    HEX_ENOENT = 2,
    HEX_EINTR = 4,
    HEX_EIO = 5,
    HEX_ENXIO = 6,
    HEX_EBADF = 9,
    HEX_EAGAIN = 11,
    HEX_ENOMEM = 12,
    HEX_EACCES = 13,
    HEX_EFAULT = 14,
    HEX_EBUSY = 16,
    HEX_EEXIST = 17,
    HEX_EXDEV = 18,
    HEX_ENODEV = 19,
    HEX_ENOTDIR = 20,
    HEX_EISDIR = 21,
    HEX_EINVAL = 22,
    HEX_ENFILE = 23,
    HEX_EMFILE = 24,
    HEX_ENOTTY = 25,
    HEX_ETXTBSY = 26,
    HEX_EFBIG = 27,
    HEX_ENOSPC = 28,
    HEX_ESPIPE = 29,
    HEX_EROFS = 30,
    HEX_EMLINK = 31,
    HEX_EPIPE = 32,
    HEX_ERANGE = 34,
    HEX_ENAMETOOLONG = 36,
    HEX_ENOSYS = 38,
    HEX_ELOOP = 40,
    HEX_EOVERFLOW = 75,
};

#endif
