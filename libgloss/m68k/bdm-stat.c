#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

/*
 * stat -- get file information
 * input parameters:
 *   0 : filename ptr
 *   1 : filename length
 *   2 : stat buf ptr
 * output parameters:
 *   0 : result
 *   1 : errno
 */


int stat (const char *filename, struct stat *buf)
{
  gdb_parambuf_t parameters;
  struct gdb_stat gbuf;
  parameters[0] = (uint32_t) filename;
  parameters[1] = (uint32_t) strlen (filename) + 1;
  parameters[2] = (uint32_t) &gbuf;
  BDM_TRAP (BDM_STAT, (uint32_t)parameters);
  convert_from_gdb_stat (&gbuf, buf);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
