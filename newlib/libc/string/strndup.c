#ifndef _REENT_ONLY

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <string.h>

char *
_DEFUN (strndup, (str, n), 
	const char *str,
	size_t n)
{
  return _strndup_r (_REENT, str, n);
}

#endif /* !_REENT_ONLY */
