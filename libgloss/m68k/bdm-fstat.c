#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

/*
 * fstat -- get file information
 * input parameters:
 *   0 : file descriptor
 *   1 : stat buf ptr
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int fstat (int fd, struct stat *buf)
{
  gdb_parambuf_t parameters;
  struct gdb_stat gbuf;
  parameters[0] = (uint32_t) fd;
  parameters[1] = (uint32_t) &gbuf;
  BDM_TRAP (BDM_FSTAT, (uint32_t)parameters);
  convert_from_gdb_stat (&gbuf, buf);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
