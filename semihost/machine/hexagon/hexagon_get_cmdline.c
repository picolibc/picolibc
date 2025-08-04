#include "hexagon_semihost.h"

int
sys_get_cmdline(char *buffer, int count)
{
    /* read a buffer from file */
    int args[] = { (int)buffer, count };
    int retval = hexagon_semihost(SYS_GET_CMDLINE, args);
    return retval;
}
