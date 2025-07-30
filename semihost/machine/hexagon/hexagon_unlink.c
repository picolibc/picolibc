#include "hexagon_semihost.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>

int
unlink(const char *name)

{
    int args[] = { (int)name, strlen(name) };
    struct hexagon_semihost_return retval = hexagon_semihost(SYS_REMOVE, args);
    if (retval.return_value == -1) {
        errno = retval.error_num;
    }
    return retval.return_value;
}
