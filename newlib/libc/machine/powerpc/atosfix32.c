/*
Copyright (c) 1984,2000 S.L. Moshier

Permission to use, copy, modify, and distribute this software for any
purpose without fee is hereby granted, provided that this entire notice
is included in all copies of any software which is or includes a copy
or modification of this software and in all copies of the supporting
documentation for such software.

THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY.  IN PARTICULAR,  THE AUTHOR MAKES NO REPRESENTATION
OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */
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
