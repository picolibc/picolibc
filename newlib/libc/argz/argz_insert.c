/* Copyright (C) 2002, 2005 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <argz.h>
#define __need_ptrdiff_t
#include <stddef.h>

error_t
_DEFUN (argz_insert, (argz, argz_len, before, entry),
       char **argz _AND
       size_t *argz_len _AND
       char *before _AND
       const char *entry)
{
  int len = 0;

  if (before == NULL)
    return argz_add(argz, argz_len, entry);

  if (before < *argz || before >= *argz + *argz_len)
    return EINVAL;

  while (before != *argz && before[-1])
    before--;

  /* delta will always be non-negative, and < *argz_len */
  ptrdiff_t delta = before - *argz;

  len = strlen(entry) + 1;

  if(!(*argz = (char *)realloc(*argz, *argz_len + len)))
    return ENOMEM;
  
  memmove(*argz + delta + len, *argz + delta,  *argz_len - delta);
  memcpy(*argz + delta, entry, len);

  *argz_len += len;

  return 0;
}
