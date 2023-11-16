#include <newlib.h>

#ifdef _FVWRITE_IN_STREAMIO

#include <reent.h>
#include <stdio.h>
#include "fvwrite.h"

/*
 * Flush out all the vectors defined by the given uio,
 * then reset it so that it can be reused.
 */
int
__sprint_r (struct _reent *ptr,
       FILE *fp,
       register struct __suio *uio)
{
	register int err = 0;

	if (uio->uio_resid == 0) {
		uio->uio_iovcnt = 0;
		return (0);
	}
	err = __sfvwrite_r(ptr, fp, uio);
	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return (err);
}

#endif
