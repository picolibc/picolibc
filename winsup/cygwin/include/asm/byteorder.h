/* asm/byteorder.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _I386_BYTEORDER_H
#define _I386_BYTEORDER_H

#include <stdint.h>
#include <machine/endian.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LITTLE_ENDIAN_BITFIELD
#define __LITTLE_ENDIAN_BITFIELD
#endif

extern uint32_t	ntohl(uint32_t);
extern uint16_t	ntohs(uint16_t);
extern uint32_t	htonl(uint32_t);
extern uint16_t	htons(uint16_t);

#define __constant_ntohl(x) \
	((uint32_t)((((uint32_t)(x) & 0x000000ffU) << 24) | \
		   (((uint32_t)(x) & 0x0000ff00U) <<  8) | \
		   (((uint32_t)(x) & 0x00ff0000U) >>  8) | \
		   (((uint32_t)(x) & 0xff000000U) >> 24)))

#define __constant_ntohs(x) \
	((uint16_t)((((uint16_t)(x) & 0x00ff) << 8) | \
		   (((uint16_t)(x) & 0xff00) >> 8))) \

#define __constant_htonl(x) __constant_ntohl(x)
#define __constant_htons(x) __constant_ntohs(x)

#if defined (__OPTIMIZE__) && !defined (__NO_INLINE__)
#  define ntohl(x) \
(__builtin_constant_p((long)(x)) ? \
 __constant_ntohl((x)) : \
 __ntohl((x)))
#  define ntohs(x) \
(__builtin_constant_p((short)(x)) ? \
 __constant_ntohs((x)) : \
 __ntohs((x)))
#  define htonl(x) \
(__builtin_constant_p((long)(x)) ? \
 __constant_htonl((x)) : \
 __htonl((x)))
#  define htons(x) \
(__builtin_constant_p((short)(x)) ? \
 __constant_htons((x)) : \
 __htons((x)))
#endif

#ifdef __cplusplus
}
#endif

#endif
