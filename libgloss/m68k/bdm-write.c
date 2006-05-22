#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <unistd.h>
#include <errno.h>

/*
 * write -- write to a file descriptor
 * input parameters:
 *   0 : file descriptor
 *   1 : buf ptr
 *   2 : count
 * output parameters:
 *   0 : result
 *   1 : errno
 */

ssize_t write (int fd, const void *buf, size_t count)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) fd;
  parameters[1] = (uint32_t) buf;
  parameters[2] = (uint32_t) count;
  BDM_TRAP (BDM_WRITE, (uint32_t)parameters);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
