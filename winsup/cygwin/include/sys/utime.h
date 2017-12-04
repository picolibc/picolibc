/* sys/utime.h

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _SYS_UTIME_H
#define _SYS_UTIME_H

#ifdef __cplusplus
extern "C" {
#endif
#include <_ansi.h>
#include <sys/types.h>

struct utimbuf
{
  time_t actime;
  time_t modtime;
};

int utime (const char *__path, const struct utimbuf *__buf);

#ifdef __cplusplus
};
#endif

#endif /* _SYS_UTIME_H */
