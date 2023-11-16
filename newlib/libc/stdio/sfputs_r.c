#include <newlib.h>

#ifndef _FVWRITE_IN_STREAMIO

#include <reent.h>
#include <stdio.h>

int
__sfputs_r (struct _reent *ptr,
       FILE *fp,
       const char *buf,
       size_t len)
{
	register int i;

	for (i = 0; i < len; i++) {
		if (_fputc_r (ptr, buf[i], fp) == EOF)
			return -1;
	}
	return (0);
}

#endif
