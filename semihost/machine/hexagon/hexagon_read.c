#include "hexagon_semihost.h"
#include <unistd.h>

ssize_t
read(int fd, void *buffer, size_t count)
{
    int args[] = { fd, (int)buffer, count };
    int retval = hexagon_semihost(SYS_READ, args);
    // hexagon_semihost(SYS_READ,...) return the number of bytes NOT read
    return count - retval;
}
