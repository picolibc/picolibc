#include "hexagon_semihost.h"
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

int
close(int fd)
{
    int arg[] = { fd };
    struct hexagon_semihost_return retval = hexagon_semihost(SYS_CLOSE, arg);
    if (retval.return_value == -1) {
        errno = retval.error_num;
    }
    return retval.return_value;
}
