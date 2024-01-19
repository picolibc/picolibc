#include <newlib.h>

#ifndef _FVWRITE_IN_STREAMIO

#include <stdio.h>
#include <wchar.h>

int
__sfputws (
       FILE *fp,
       const wchar_t *buf,
       size_t len)
{
	register size_t i;

	for (i = 0; i < len; i++) {
		if (fputwc (buf[i], fp) == WEOF)
			return -1;
	}
	return (0);
}

#endif
