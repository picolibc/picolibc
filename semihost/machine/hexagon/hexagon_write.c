#include "hexagon_semihost.h"
#include <unistd.h>

ssize_t
write(int fd, const void *buffer, size_t count)
{
    int args[] = { fd, (int)buffer, count };
    int retval = hexagon_semihost(SYS_WRITE, args);
    if (retval == -1) {
        return retval;
    } else {
        return count - retval;
    }
}
