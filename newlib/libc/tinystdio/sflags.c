/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* No user fns here. Pesch 15apr92 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "stdio_private.h"

/*
 * Return the (stdio) flags for a given mode.  Store the flags
 * to be passed to an open() syscall through *optr.
 * Return 0 on error.
 */

int
__posix_sflags (const char *mode, int *optr)
{
  int ret, m, o;

  switch (mode[0])
    {
    case 'r':			/* open for reading */
      ret = __SRD;
      m = O_RDONLY;
      o = 0;
      break;

    case 'w':			/* open for writing */
      ret = __SWR;
      m = O_WRONLY;
      o = O_CREAT | O_TRUNC;
      break;

    case 'a':			/* open for appending */
      ret = __SWR;
      m = O_WRONLY;
      o = O_CREAT | O_APPEND;
      break;
    default:			/* illegal mode */
      errno = EINVAL;
      return (0);
    }
  while (*++mode)
    {
      switch (*mode)
	{
	case '+':
	  ret |= (__SRD | __SWR);
	  m = (m & ~O_ACCMODE) | O_RDWR;
	  break;
	default:
	  break;
	}
    }
  *optr = m | o;
  return ret;
}
