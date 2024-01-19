#include <newlib.h>

#ifdef _FVWRITE_IN_STREAMIO

#include <stdio.h>
#include <wchar.h>
#include "fvwrite.h"

/*
 * Flush out all the vectors defined by the given uio,
 * then reset it so that it can be reused.
 */
int
__swprint (
       FILE *fp,
       register struct __suio *uio)
{
	register int err = 0;
	struct __siov *iov;
	wchar_t *p;
	int i, len;

	if (uio->uio_resid == 0) {
		uio->uio_iovcnt = 0;
		return (0);
	}
	iov = uio->uio_iov;
	for (; uio->uio_resid != 0;
	     uio->uio_resid -= len, iov++) {
		p = (wchar_t *) iov->iov_base;
		len = iov->iov_len;
		for (i = 0; i < len; i++) {
			if (fputwc (p[i], fp) == WEOF) {
				err = -1;
				goto out;
			}
		}
	}
out:
	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return (err);
}

#endif
