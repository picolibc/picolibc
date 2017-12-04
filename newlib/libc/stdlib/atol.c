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

long
_atol_r (struct _reent *ptr, const char *s)
{
  return _strtol_r (ptr, s, NULL, 10);
}

