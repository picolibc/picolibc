#include "hexagon_semihost.h"
#include <unistd.h>

int
close(int fd)
{
    int arg[] = { fd };
    int retval = hexagon_semihost(SYS_CLOSE, arg);
    return retval;
}
