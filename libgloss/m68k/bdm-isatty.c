#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <unistd.h>
#include <errno.h>

/*
 * isatty -- check if fd is a terminal
 * input parameters:
 *   0 : file descriptor
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int isatty (int fd)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) fd;
  BDM_TRAP (BDM_ISATTY, (uint32_t)parameters);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
