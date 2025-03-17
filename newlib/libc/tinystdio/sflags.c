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
 * Return the (stdio) flags for a given mode. Store the flags to be
 * passed to open() through *optr.
 *
 * Return 0 on error.
 */

int
__stdio_flags (const char *mode, int *optr)
{
  int ret;
  int o;

  switch (mode[0])
    {
    case 'r':			/* open for reading */
      ret = __SRD;
      o = O_RDONLY;
      break;

    case 'w':			/* open for writing */
      ret = __SWR;
      o = O_WRONLY | O_CREAT | O_TRUNC;
      break;

    case 'a':			/* open for appending */
      ret = __SWR;
      o = O_WRONLY | O_CREAT | O_APPEND;
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
	  o = (o & ~O_ACCMODE) | O_RDWR;
	  break;
	default:
	  break;
	}
    }
  *optr = o;
  return ret;
}
