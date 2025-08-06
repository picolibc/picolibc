#include "hexagon_semihost.h"

int
flen(int fd)
{
    int args[] = { fd };
    int retval = hexagon_semihost(SYS_FLEN, args);
    return retval;
}
