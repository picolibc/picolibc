#include <newlib.h>

#ifndef _FVWRITE_IN_STREAMIO

#include <reent.h>
#include <stdio.h>
#include <wchar.h>

int
__sfputws_r (struct _reent *ptr,
       FILE *fp,
       const wchar_t *buf,
       size_t len)
{
	register int i;

	for (i = 0; i < len; i++) {
		if (_fputwc_r (ptr, buf[i], fp) == WEOF)
			return -1;
	}
	return (0);
}

#endif
