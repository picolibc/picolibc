
#include <picolibc.h>

#ifndef __FVWRITE_IN_STREAMIO

#include <stdio.h>
#include <wchar.h>
#include "local.h"

int
__ssputws (
	FILE *fp,
	const wchar_t *buf,
	size_t len)
{
	return __ssputs (fp, (const char *) buf, len * sizeof (wchar_t));
}

#endif
