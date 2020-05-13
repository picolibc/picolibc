/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
 * Andy Wilson, 2-Oct-89.
 */

#include <stdlib.h>
#include <_ansi.h>

#ifndef _REENT_ONLY
long
atol (const char *s)
{
  return strtol (s, NULL, 10);
}
#endif /* !_REENT_ONLY */
