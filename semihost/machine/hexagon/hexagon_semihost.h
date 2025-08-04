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

#endif
