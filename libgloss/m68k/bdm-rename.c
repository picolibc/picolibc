#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/*
 * rename -- rename a file
 * input parameters:
 *   0 : oldname ptr
 *   1 : oldname length
 *   2 : newname ptr
 *   3 : newname length
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int _rename (const char *oldpath, const char *newpath)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) oldpath;
  parameters[1] = (uint32_t) strlen (oldpath) + 1;
  parameters[2] = (uint32_t) newpath;
  parameters[3] = (uint32_t) strlen (newpath) + 1;
  BDM_TRAP (BDM_RENAME, (uint32_t)parameters);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
