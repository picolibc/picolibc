#define _GNU_SOURCE
#include <sys/cdefs.h>
#include <ssp/ssp.h>
#include <string.h>

#undef mempcpy

void *__mempcpy_chk(void * __restrict, const void * __restrict, size_t, size_t);

void *
__mempcpy_chk(void * __restrict dst, const void * __restrict src, size_t len,
    size_t slen)
{
	if (len > slen)
		__chk_fail();

	if (__ssp_overlap((const char *)src, (const char *)dst, len))
		__chk_fail();

	return mempcpy(dst, src, len);
}
