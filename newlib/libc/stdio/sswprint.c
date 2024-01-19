#include <newlib.h>

#ifdef _FVWRITE_IN_STREAMIO

#include <stdio.h>
#include <wchar.h>
#include "fvwrite.h"

extern int __ssputs (FILE *fp, const char *buf,
		       size_t len);

int
__sswprint (
	FILE *fp,
	register struct __suio *uio)
{
	register struct __siov *iov = uio->uio_iov;
	register size_t len;
	int ret = 0;

	while (uio->uio_resid > 0 && uio->uio_iovcnt-- > 0) {
		if ((len = iov->iov_len) > 0) {
			if (__ssputs (fp, iov->iov_base,
					len * sizeof (wchar_t)) == EOF) {
				ret = -1;
				break;
			}
			uio->uio_resid -= len;	/* pretend we copied all */
		}
		iov++;
	}
	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return ret;
}

#endif
