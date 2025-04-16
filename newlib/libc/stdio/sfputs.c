
#include <picolibc.h>

#ifndef __FVWRITE_IN_STREAMIO

#include <stdio.h>
#include "local.h"

int
__sfputs (
       FILE *fp,
       const char *buf,
       size_t len)
{
	register size_t i;

	for (i = 0; i < len; i++) {
		if (fputc (buf[i], fp) == EOF)
			return -1;
	}
	return (0);
}

#endif
