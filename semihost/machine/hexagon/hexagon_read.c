#include "hexagon_semihost.h"
#include <errno.h>
#include <unistd.h>

ssize_t
read(int fd, void *buffer, size_t count)
{
    int args[] = { fd, (int)buffer, count };
    struct hexagon_semihost_return retval = hexagon_semihost(SYS_READ, args);
    if (retval.return_value == -1) {
        errno = retval.error_num;
    }
    // hexagon_semihost(SYS_READ,...) return the number of bytes NOT read
    return count - retval.return_value;
}
