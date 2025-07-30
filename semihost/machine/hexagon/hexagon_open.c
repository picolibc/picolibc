#include "fcntl.h"
#include "hexagon_semihost.h"
#include <errno.h>
#include <string.h>

#define O_BINARY 0

#define MODEMAP_LENGTH 14

static const int modemap[MODEMAP_LENGTH] = {
    O_RDONLY,
    O_RDONLY | O_BINARY,
    O_RDWR,
    O_RDWR | O_BINARY,

    O_WRONLY | O_CREAT | O_TRUNC,
    O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,
    O_RDWR | O_CREAT | O_TRUNC,
    O_RDWR | O_CREAT | O_TRUNC | O_BINARY,

    O_WRONLY | O_CREAT | O_APPEND,
    O_WRONLY | O_CREAT | O_APPEND | O_BINARY,
    O_RDWR | O_CREAT | O_APPEND,
    O_RDWR | O_CREAT | O_APPEND | O_BINARY,

    O_RDWR | O_CREAT,
    O_RDWR | O_CREAT | O_EXCL,
};

int
open(const char *name, int mode, ...)
{
    mode &= O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_TRUNC | O_APPEND |
            O_EXCL | O_BINARY;
    int semimode = 0;
    for (size_t i = 0; i < MODEMAP_LENGTH; ++i) {
        if (modemap[i] == mode) {
            semimode = i;
            break;
        }
    }

    size_t length = strlen(name);
    int args[] = { (int)name, semimode, length };
    struct hexagon_semihost_return retval = hexagon_semihost(SYS_OPEN, args);
    if (retval.return_value == -1) {
        errno = retval.error_num;
    }
    return retval.return_value;
}
