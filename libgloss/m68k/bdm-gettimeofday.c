#include "bdm-semihost.h"
#include "bdm-gdb.h"
#include <sys/time.h>
#include <errno.h>

/*
 * gettimeofday -- get the current time
 * input parameters:
 *   0 : timeval ptr
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int gettimeofday (struct timeval *tv, struct timezone *tz)
{
  gdb_parambuf_t parameters;
  struct gdb_timeval gtv;
  if (!tv)
    return 0;
  if (tz)
    {
      errno = EINVAL;
      return -1;
    }
  parameters[0] = (uint32_t) &gtv;
  BDM_TRAP (BDM_GETTIMEOFDAY, (uint32_t)parameters);
  convert_from_gdb_timeval (&gtv, tv);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
