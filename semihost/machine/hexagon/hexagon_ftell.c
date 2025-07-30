#include "hexagon_semihost.h"
#include <errno.h>

int
hexagon_ftell(int fd)
{
    int args[] = { fd };
    struct hexagon_semihost_return retval = hexagon_semihost(SYS_FTELL, args);
    if (retval.return_value == -1) {
        errno = retval.error_num;
    }
    return retval.return_value;
}
