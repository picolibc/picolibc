#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <unistd.h>
#include <errno.h>

/*
 * close -- close a file descriptor.
 * input parameters:
 *   0 : file descriptor
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int close (int fd)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) fd;
  BDM_TRAP (BDM_CLOSE, (uint32_t)parameters);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
