/* Copyright (c) 2004 Jeff Johnston  <jjohnstn@redhat.com> */
#ifndef __MACHINE_ENDIAN_H__
#define	__MACHINE_ENDIAN_H__

#include <sys/cdefs.h>
#include <sys/_types.h>
#include <machine/_endian.h>

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define	_QUAD_HIGHWORD	1
#define	_QUAD_LOWWORD	0
#else
#define	_QUAD_HIGHWORD	0
#define	_QUAD_LOWWORD	1
#endif

#if __BSD_VISIBLE
#define	LITTLE_ENDIAN	_LITTLE_ENDIAN
#define	BIG_ENDIAN	_BIG_ENDIAN
#define	PDP_ENDIAN	_PDP_ENDIAN
#define	BYTE_ORDER	_BYTE_ORDER
#endif

#ifdef __GNUC__
#define	__bswap16(_x)	__builtin_bswap16(_x)
#define	__bswap32(_x)	__builtin_bswap32(_x)
#define	__bswap64(_x)	__builtin_bswap64(_x)
#else /* __GNUC__ */
static __inline __uint16_t
__bswap16(__uint16_t _x)
{

	return ((__uint16_t)((_x >> 8) | ((_x << 8) & 0xff00)));
}

static __inline __uint32_t
__bswap32(__uint32_t _x)
{

	return ((__uint32_t)((_x >> 24) | ((_x >> 8) & 0xff00) |
	    ((_x << 8) & 0xff0000) | ((_x << 24) & 0xff000000)));
}

static __inline __uint64_t
__bswap64(__uint64_t _x)
{

	return ((__uint64_t)((_x >> 56) | ((_x >> 40) & 0xff00) |
	    ((_x >> 24) & 0xff0000) | ((_x >> 8) & 0xff000000) |
	    ((_x << 8) & ((__uint64_t)0xff << 32)) |
	    ((_x << 24) & ((__uint64_t)0xff << 40)) |
	    ((_x << 40) & ((__uint64_t)0xff << 48)) | ((_x << 56))));
}
#endif /* !__GNUC__ */

/* endian(3) - similar to linux <endian.h> */
#if _BYTE_ORDER == _LITTLE_ENDIAN
#define htobe16(_x) __bswap16(_x)
#define htole16(_x) ((__uint16_t)(_x))
#define be16toh(_x) __bswap16(_x)
#define le16toh(_x) ((__uint16_t)(_x))
#define htobe32(_x) __bswap32(_x)
#define htole32(_x) ((__uint32_t)(_x))
#define be32toh(_x) __bswap32(_x)
#define le32toh(_x) ((__uint32_t)(_x))
#define htobe64(_x) __bswap64(_x)
#define htole64(_x) ((__uint64_t)(_x))
#define be64toh(_x) __bswap64(_x)
#define le64toh(_x) ((__uint64_t)(_x))
#else
#define htobe16(_x) ((__uint16_t)(_x))
#define htole16(_x) __bswap16(_x)
#define be16toh(_x) ((__uint16_t)(_x))
#define le16toh(_x) __bswap16(_x)
#define htobe32(_x) ((__uint32_t)(_x))
#define htole32(_x) __bswap32(_x)
#define be32toh(_x) ((__uint32_t)(_x))
#define le32toh(_x) __bswap32(_x)
#define htobe64(_x) ((__uint64_t)(_x))
#define htole64(_x) __bswap64(_x)
#define be64toh(_x) ((__uint64_t)(_x))
#define le64toh(_x) __bswap64(_x)
#endif

#endif /* __MACHINE_ENDIAN_H__ */
