#include <newlib.h>

#ifndef _FVWRITE_IN_STREAMIO

#include <reent.h>
#include <stdio.h>
#include <wchar.h>

extern int __ssputs_r (struct _reent *ptr, FILE *fp, const char *buf,
		       size_t len);

int
__ssputws_r (struct _reent *ptr,
	FILE *fp,
	const wchar_t *buf,
	size_t len)
{
	return __ssputs_r (ptr, fp, (const char *) buf, len * sizeof (wchar_t));
}

#endif
