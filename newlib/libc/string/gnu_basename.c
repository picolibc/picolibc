#ifndef _NO_BASENAME
/* Copyright 2015 Red Hat, Inc.
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/* The differences with the POSIX version (unix/basename.c):
 * - declared in <string.h> (instead of <libgen.h>);
 * - the argument is never modified, and therefore is marked const;
 * - the empty string is returned if path is an empty string, "/", or ends
 *   with a trailing slash.
 */

#include <string.h>

char *
__gnu_basename (const char *path)
{
  char *p;
  if ((p = strrchr (path, '/')))
    return p + 1;
  return (char *) path;
}

#endif /* !_NO_BASENAME  */
