#ifndef _UCHAR_H
#define	_UCHAR_H

#include <sys/cdefs.h>
#include <wchar.h>

/* Either C2x or if C++ doesn't already define char8_t */
#if __ISO_C_VISIBLE >= 2020 && !defined (__cpp_char8_t)
typedef unsigned char		char8_t;
#endif

/* C++11 already defines those types. */
#if !defined (__cplusplus) || (__cplusplus - 0 < 201103L)
typedef	__uint_least16_t	char16_t;
typedef	__uint_least32_t	char32_t;
#endif

__BEGIN_DECLS

/* Either C2x or if C++ defines char8_t */
#if __ISO_C_VISIBLE >= 2020 || defined (__cpp_char8_t)
size_t  c8rtomb(char * __restrict, char8_t, mbstate_t * __restrict);
size_t	mbrtoc8(char8_t * __restrict, const char * __restrict, size_t,
		mbstate_t * __restrict);
#endif

size_t	c16rtomb(char * __restrict, char16_t, mbstate_t * __restrict);
size_t	mbrtoc16(char16_t * __restrict, const char * __restrict, size_t,
		 mbstate_t * __restrict);

size_t	c32rtomb(char * __restrict, char32_t, mbstate_t * __restrict);
size_t	mbrtoc32(char32_t * __restrict, const char * __restrict, size_t,
		 mbstate_t * __restrict);

__END_DECLS

#endif /* _UCHAR_H */
