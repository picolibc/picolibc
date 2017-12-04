/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* No user fns here. Pesch 15apr92 */

#include <_ansi.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

/*
 * Return the (stdio) flags for a given mode.  Store the flags
 * to be passed to an open() syscall through *optr.
 * Return 0 on error.
 */

int
__sflags (struct _reent *ptr,
       register char *mode,
       int *optr)
{
  register int ret, m, o;

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
      ret = __SWR | __SAPP;
      m = O_WRONLY;
      o = O_CREAT | O_APPEND;
      break;
    default:			/* illegal mode */
      ptr->_errno = EINVAL;
      return (0);
    }
  while (*++mode)
    {
      switch (*mode)
	{
	case '+':
	  ret = (ret & ~(__SRD | __SWR)) | __SRW;
	  m = (m & ~O_ACCMODE) | O_RDWR;
	  break;
	case 'b':
#ifdef O_BINARY
	  m |= O_BINARY;
#endif
	  break;
#ifdef __CYGWIN__
	case 't':
	  m |= O_TEXT;
	  break;
#endif
#if defined (O_CLOEXEC) && defined (_GLIBC_EXTENSION)
	case 'e':
	  m |= O_CLOEXEC;
	  break;
#endif
	case 'x':
	  m |= O_EXCL;
	  break;
	default:
	  break;
	}
    }
#if defined (O_TEXT) && !defined (__CYGWIN__)
  if (!(m | O_BINARY))
    m |= O_TEXT;
#endif
  *optr = m | o;
  return ret;
}
