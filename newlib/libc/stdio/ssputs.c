#include <newlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int
__ssputs (
	FILE *fp,
	const char *buf,
	size_t len)
{
	register unsigned int w;

	w = (unsigned int) fp->_w;
	if (len >= w && fp->_flags & (__SMBF | __SOPT)) {
		/* must be asprintf family */
		unsigned char *str;
		int curpos = (fp->_p - fp->_bf._base);
		/* Choose a geometric growth factor to avoid
		 * quadratic realloc behavior, but use a rate less
		 * than (1+sqrt(5))/2 to accomodate malloc
		 * overhead. asprintf EXPECTS us to overallocate, so
		 * that it can add a trailing \0 without
		 * reallocating.  The new allocation should thus be
		 * max(prev_size*1.5, curpos+len+1). */
		unsigned int newsize = fp->_bf._size * 3 / 2;
		if (newsize < curpos + len + 1)
			newsize = curpos + len + 1;
		if (fp->_flags & __SOPT)
		{
			/* asnprintf leaves original buffer alone.  */
			str = (unsigned char *)malloc (newsize);
			if (!str)
				goto err;
			memcpy (str, fp->_bf._base, curpos);
			fp->_flags = (fp->_flags & ~__SOPT) | __SMBF;
		}
		else
		{
			str = (unsigned char *)realloc (fp->_bf._base,
					newsize);
			if (!str) {
				/* Free unneeded buffer.  */
				free (fp->_bf._base);
				goto err;
			}
		}
		fp->_bf._base = str;
		fp->_p = str + curpos;
		fp->_bf._size = newsize;
		w = len;
		fp->_w = newsize - curpos;
	}
	if (len < w)
		w = len;
	memmove ((void *) fp->_p, (void *) buf, (size_t) (w));
	fp->_w -= w;
	fp->_p += w;

	return 0;

err:
	_REENT_ERRNO(ptr) = ENOMEM;
	fp->_flags |= __SERR;
	return EOF;
}
