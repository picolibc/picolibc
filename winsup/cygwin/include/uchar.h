#ifndef _UCHAR_H
#define	_UCHAR_H

#include <sys/cdefs.h>
#include <wchar.h>

typedef	__uint16_t	char16_t;
typedef	__uint32_t	char32_t;

__BEGIN_DECLS

size_t	c16rtomb(char * __restrict, char16_t, mbstate_t * __restrict);
size_t	mbrtoc16(char16_t * __restrict, const char * __restrict, size_t,
		 mbstate_t * __restrict);

size_t	c32rtomb(char * __restrict, char32_t, mbstate_t * __restrict);
size_t	mbrtoc32(char32_t * __restrict, const char * __restrict, size_t,
		 mbstate_t * __restrict);

__END_DECLS

#endif /* _UCHAR_H */
