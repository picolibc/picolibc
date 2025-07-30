#include "hexagon_semihost.h"
#include <errno.h>

int
flen(int fd)
{
    int args[] = { fd };
    struct hexagon_semihost_return retval = hexagon_semihost(SYS_FLEN, args);
    if (retval.return_value == -1) {
        errno = retval.error_num;
    }
    return retval.return_value;
}
