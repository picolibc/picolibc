#include <newlib.h>

#ifndef _FVWRITE_IN_STREAMIO

#include <stdio.h>
#include <wchar.h>

extern int __ssputs (FILE *fp, const char *buf,
		       size_t len);

int
__ssputws (
	FILE *fp,
	const wchar_t *buf,
	size_t len)
{
	return __ssputs (fp, (const char *) buf, len * sizeof (wchar_t));
}

#endif
