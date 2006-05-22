#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/*
 * open -- Open a file.
 * input parameters:
 *   0 : fname ptr
 *   1 : fname length
 *   2 : flags
 *   3 : mode
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int open (const char *fname, int flags, ...)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) fname;
  parameters[1] = strlen (fname) + 1;
  parameters[2] = convert_to_gdb_open_flags (flags);
  if (flags & O_CREAT)
    {
      va_list ap;
      va_start (ap, flags);
      parameters[3] = convert_to_gdb_mode_t (va_arg (ap, mode_t));
      va_end (ap);
    }
  else
    parameters[3] = 0;
  BDM_TRAP (BDM_OPEN, (uint32_t)parameters);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
