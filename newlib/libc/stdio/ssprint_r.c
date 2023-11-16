#include <newlib.h>

#ifdef _FVWRITE_IN_STREAMIO

#include <reent.h>
#include <stdio.h>
#include "fvwrite.h"

extern int __ssputs_r (struct _reent *ptr, FILE *fp, const char *buf,
		       size_t len);

int
__ssprint_r (struct _reent *ptr,
	FILE *fp,
	register struct __suio *uio)
{
	register struct __siov *iov = uio->uio_iov;
	register size_t len;
	int ret = 0;

	while (uio->uio_resid > 0 && uio->uio_iovcnt-- > 0) {
		if ((len = iov->iov_len) > 0) {
			if (__ssputs_r (ptr, fp, iov->iov_base, len) == EOF) {
				ret = EOF;
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
