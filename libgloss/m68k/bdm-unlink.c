#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/*
 * unlink -- unlink (delete) a file
 * input parameters:
 *   0 : filename ptr
 *   1 : filename length
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int unlink (const char *path)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) path;
  parameters[1] = (uint32_t) strlen (path) + 1;
  BDM_TRAP (BDM_UNLINK, (uint32_t)parameters);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
