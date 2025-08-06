#include "hexagon_semihost.h"
#include <string.h>
#include <unistd.h>

int
unlink(const char *name)
{
    int args[] = { (int)name, strlen(name) };
    int retval = hexagon_semihost(SYS_REMOVE, args);
    return retval;
}
