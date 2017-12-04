/*
 * Jeff Johnston - 02/13/2002
 */

#ifdef __SPE__

#include <stdlib.h>
#include <_ansi.h>

__int32_t
_atosfix32_r (struct _reent *reent,
	const char *s)
{
  return _strtosfix32_r (reent, s, NULL);
}

#ifndef _REENT_ONLY
__int32_t
atosfix32 (const char *s)
{
  return strtosfix32 (s, NULL);
}

#endif /* !_REENT_ONLY */

#endif /* __SPE__ */
