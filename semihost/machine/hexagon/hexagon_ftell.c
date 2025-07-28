#include "hexagon_semihost.h"

int
hexagon_ftell(int fd)
{
    int args[] = { fd };
    int retval = hexagon_semihost(SYS_FTELL, args);
    return retval;
}
