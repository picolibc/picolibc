#include "hexagon_semihost.h"
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

ssize_t
write(int fd, const void *buffer, size_t count)
{
    int args[] = { fd, (int)buffer, count };
    struct hexagon_semihost_return retval = hexagon_semihost(SYS_WRITE, args);
    if (retval.return_value == -1) {
        errno = retval.error_num;
        return retval.return_value;
    }
    return count - retval.return_value;
}
