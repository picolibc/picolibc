#include "hexagon_semihost.h"

int
sys_get_cmdline(char *buffer, int count)
{ /* read a buffer from file */
    int args[] = { (int)buffer, count };
    struct hexagon_semihost_return retval =
        hexagon_semihost(SYS_GET_CMDLINE, args);
    return retval.return_value;
}
