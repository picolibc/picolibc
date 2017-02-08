/* endian.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <sys/config.h>
#include <machine/endian.h>

/*#ifdef  __USE_BSD*/
# ifndef LITTLE_ENDIAN
#  define LITTLE_ENDIAN __LITTLE_ENDIAN
# endif
# ifndef BIG_ENDIAN
#  define BIG_ENDIAN    __BIG_ENDIAN
# endif
# ifndef PDP_ENDIAN
#  define PDP_ENDIAN    __PDP_ENDIAN
# endif
# ifndef BYTE_ORDER
#  define BYTE_ORDER    __BYTE_ORDER
# endif
/*#endif*/

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define __LONG_LONG_PAIR(HI, LO) LO, HI
#elif __BYTE_ORDER == __BIG_ENDIAN
# define __LONG_LONG_PAIR(HI, LO) HI, LO
#endif

#if __BSD_VISIBLE

#include <bits/byteswap.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN

#define htobe16(x) __bswap_16(x)
#define htobe32(x) __bswap_32(x)
#define htobe64(x) __bswap_64(x)

#define be16toh(x) __bswap_16(x)
#define be32toh(x) __bswap_32(x)
#define be64toh(x) __bswap_64(x)

#define htole16(x) (x)
#define htole32(x) (x)
#define htole64(x) (x)

#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)

#endif /*__BYTE_ORDER == __LITTLE_ENDIAN*/

#if __BYTE_ORDER == __BIG_ENDIAN

#define htobe16(x) (x)
#define htobe32(x) (x)
#define htobe64(x) (x)

#define be16toh(x) (x)
#define be32toh(x) (x)
#define be64toh(x) (x)

#define htole16(x) __bswap_16(x)
#define htole32(x) __bswap_32(x)
#define htole64(x) __bswap_64(x)

#define le16toh(x) __bswap_16(x)
#define le32toh(x) __bswap_32(x)
#define le64toh(x) __bswap_64(x)

#endif /*__BYTE_ORDER == __BIG_ENDIAN*/

#endif /*__BSD_VISIBLE*/

#endif /*_ENDIAN_H_*/

