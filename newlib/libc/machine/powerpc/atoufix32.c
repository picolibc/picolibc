/*
 * Jeff Johnston - 02/13/2002
 */

#ifdef __SPE__ 

#include <stdlib.h>
#include <_ansi.h>

__uint32_t
_atoufix32_r (struct _reent *reent,
	const char *s)
{
  return _strtoufix32_r (reent, s, NULL);
}

#ifndef _REENT_ONLY
__uint32_t
atoufix32 (const char *s)
{
  return strtoufix32 (s, NULL);
}

#endif /* !_REENT_ONLY */

#endif /* __SPE__ */
