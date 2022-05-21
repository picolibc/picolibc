/* Copyright (c) 2004 Jeff Johnston  <jjohnstn@redhat.com> */
#ifndef __ARPA_INET_H__
#define __ARPA_INET_H__

#include <endian.h>

/* byteorder(3) - simimlar to linux <arpa/inet.h> */
#ifndef __machine_host_to_from_network_defined
#if _BYTE_ORDER == _LITTLE_ENDIAN
#define	__htonl(_x)	__bswap32(_x)
#define	__htons(_x)	__bswap16(_x)
#define	__ntohl(_x)	__bswap32(_x)
#define	__ntohs(_x)	__bswap16(_x)
#define	htonl(_x)	__htonl(_x)
#define	htons(_x)	__htons(_x)
#define	ntohl(_x)	__htonl(_x)
#define	ntohs(_x)	__htons(_x)
#else
#define	__htonl(_x)	((__uint32_t)(_x))
#define	__htons(_x)	((__uint16_t)(_x))
#define	__ntohl(_x)	((__uint32_t)(_x))
#define	__ntohs(_x)	((__uint16_t)(_x))
#define	htonl(_x)	__htonl(_x)
#define	htons(_x)	__htons(_x)
#define	ntohl(_x)	__ntohl(_x)
#define	ntohs(_x)	__ntohs(_x)
#endif
#endif /* __machine_host_to_from_network_defined */

#endif /* __ARPA_INET_H__ */
