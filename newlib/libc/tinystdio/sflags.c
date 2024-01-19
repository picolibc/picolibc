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

#include "stdio_private.h"

/*
 * In POSIX_IO mode, this is called __posix_sflags and __stdio_sflags
 * is a static inline on top of this. Otherwise, this is called
 * __stdio_sflags.
 *
 * Return the (stdio) flags for a given mode. When POSIX_IO is
 * available, also store the flags to be passed to an open() syscall
 * through *optr.
 *
 * Return 0 on error.
 */

#ifdef POSIX_IO
int
__posix_sflags (const char *mode, int *optr)
#else
int
__stdio_sflags (const char *mode) 
#endif
{
  int ret;
#ifdef POSIX_IO
  int m, o;
#endif

  switch (mode[0])
    {
    case 'r':			/* open for reading */
      ret = __SRD;
#ifdef POSIX_IO
      m = O_RDONLY;
      o = 0;
#endif
      break;

    case 'w':			/* open for writing */
      ret = __SWR;
#ifdef POSIX_IO
      m = O_WRONLY;
      o = O_CREAT | O_TRUNC;
#endif
      break;

    case 'a':			/* open for appending */
      ret = __SWR;
#ifdef POSIX_IO
      m = O_WRONLY;
      o = O_CREAT | O_APPEND;
#endif
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
#ifdef POSIX_IO
	  m = (m & ~O_ACCMODE) | O_RDWR;
#endif
	  break;
	default:
	  break;
	}
    }
#ifdef POSIX_IO
  *optr = m | o;
#endif
  return ret;
}
