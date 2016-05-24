/* machine/_endian.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __MACHINE_ENDIAN_H__
#error "must be included via <machine/endian.h>"
#endif /* !__MACHINE_ENDIAN_H__ */

#include <_ansi.h>
#include <bits/endian.h>

#define _LITTLE_ENDIAN __LITTLE_ENDIAN
#define _BIG_ENDIAN    __BIG_ENDIAN
#define _PDP_ENDIAN    __PDP_ENDIAN
#define _BYTE_ORDER    __BYTE_ORDER

#define __machine_host_to_from_network_defined

_ELIDABLE_INLINE __uint32_t __ntohl(__uint32_t);
_ELIDABLE_INLINE __uint16_t __ntohs(__uint16_t);

_ELIDABLE_INLINE __uint32_t
__ntohl(__uint32_t _x)
{
	__asm__("bswap %0" : "=r" (_x) : "0" (_x));
	return _x;
}

_ELIDABLE_INLINE __uint16_t
__ntohs(__uint16_t _x)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/
		: "=Q" (_x)
		:  "0" (_x));
	return _x;
}

#define __htonl(_x) __ntohl(_x)
#define __htons(_x) __ntohs(_x)
