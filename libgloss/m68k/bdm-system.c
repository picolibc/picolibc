#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * system: execute command on (remote) host
 * input parameters:
 *   0 : command ptr
 *   1 : command length
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int _system (const char *command)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) command;
  parameters[1] = (uint32_t) strlen (command) + 1;
  BDM_TRAP (BDM_SYSTEM, (uint32_t)parameters);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
