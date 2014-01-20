/* cygwin/in6.h

   Copyright 2006, 2007, 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* NOTE:  This file is NOT for direct inclusion.  Use netinet/in.h. */

#ifndef _CYGWIN_IN6_H
#define _CYGWIN_IN6_H

#define INET6_ADDRSTRLEN 46

#define IN6_ARE_ADDR_EQUAL(a, b) \
	(((const uint32_t *)(a))[0] == ((const uint32_t *)(b))[0] \
	 && ((const uint32_t *)(a))[1] == ((const uint32_t *)(b))[1] \
	 && ((const uint32_t *)(a))[2] == ((const uint32_t *)(b))[2] \
	 && ((const uint32_t *)(a))[3] == ((const uint32_t *)(b))[3])

#define IN6_IS_ADDR_UNSPECIFIED(addr) \
	(((const uint32_t *)(addr))[0] == 0 \
	 && ((const uint32_t *)(addr))[1] == 0 \
	 && ((const uint32_t *)(addr))[2] == 0 \
	 && ((const uint32_t *)(addr))[3] == 0)

#define IN6_IS_ADDR_LOOPBACK(addr) \
	(((const uint32_t *)(addr))[0] == 0 \
	 && ((const uint32_t *)(addr))[1] == 0 \
	 && ((const uint32_t *)(addr))[2] == 0 \
	 && ((const uint32_t *)(addr))[3] == htonl (1))

#define IN6_IS_ADDR_MULTICAST(addr) (((const uint8_t *) (addr))[0] == 0xff)

#define IN6_IS_ADDR_LINKLOCAL(addr) \
	((((const uint16_t *)(addr))[0] & htons (0xffc0)) == htons (0xfe80))

#define IN6_IS_ADDR_SITELOCAL(addr) \
	((((const uint16_t *)(addr))[0] & htons (0xffc0)) == htons (0xfec0))

#define IN6_IS_ADDR_V4MAPPED(addr) \
	(((const uint32_t *)(addr))[0] == 0 \
	 && ((const uint32_t *)(addr))[1] == 0 \
	 && ((const uint32_t *)(addr))[2] == htonl (0xffff))

#define IN6_IS_ADDR_V4COMPAT(addr) \
	(((const uint32_t *)(addr))[0] == 0 \
	 && ((const uint32_t *)(addr))[1] == 0 \
	 && ((const uint32_t *)(addr))[2] == 0 \
	 && ntohl (((const uint32_t *)(addr))[3]) > 1)

#define IN6_IS_ADDR_MC_NODELOCAL(addr) \
	(IN6_IS_ADDR_MULTICAST(addr) \
	 && (((const uint8_t *)(addr))[1] & 0xf) == 0x1)

#define IN6_IS_ADDR_MC_LINKLOCAL(addr) \
	(IN6_IS_ADDR_MULTICAST (addr) \
	 && (((const uint8_t *)(addr))[1] & 0xf) == 0x2)

#define IN6_IS_ADDR_MC_SITELOCAL(addr) \
	(IN6_IS_ADDR_MULTICAST(addr) \
	 && (((const uint8_t *)(addr))[1] & 0xf) == 0x5)

#define IN6_IS_ADDR_MC_ORGLOCAL(addr) \
	(IN6_IS_ADDR_MULTICAST(addr) \
	 && (((const uint8_t *)(addr))[1] & 0xf) == 0x8)

#define IN6_IS_ADDR_MC_GLOBAL(addr) \
	(IN6_IS_ADDR_MULTICAST(addr) \
	 && (((const uint8_t *)(addr))[1] & 0xf) == 0xe)

struct in6_addr
{
  union
    {
      uint8_t 	  __s6_addr[16];
      uint16_t 	  __s6_addr16[8];
      uint32_t 	  __s6_addr32[4];
    } __u6;
#define s6_addr		__u6.__s6_addr
#define s6_addr16	__u6.__s6_addr16
#define s6_addr32	__u6.__s6_addr32
};

struct ipv6_mreq
{
  struct in6_addr ipv6mr_multiaddr;
  uint32_t        ipv6mr_interface;
};

struct ipv6_rt_hdr {
  uint8_t	  nexthdr;
  uint8_t	  hdrlen;
  uint8_t	  type;
  uint8_t	  segments_left;
  /* type specific data, variable length */
};

struct in6_pktinfo
{
  struct in6_addr ipi6_addr;
  uint32_t        ipi6_ifindex;
};

#if defined (__INSIDE_CYGWIN__) && !defined (_CYGWIN_IN_H)
typedef uint16_t in_port_t;
#endif

struct sockaddr_in6
{
  sa_family_t	  sin6_family;		/* AF_INET6 */
  in_port_t	  sin6_port;		/* Port number. */
  uint32_t	  sin6_flowinfo;	/* Traffic class and flow inf. */
  struct in6_addr sin6_addr;		/* IPv6 address. */
  uint32_t	  sin6_scope_id;	/* Set of interfaces for a scope. */
};

#define IN6ADDR_ANY_INIT	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
#define IN6ADDR_LOOPBACK_INIT	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

#endif	/* _CYGWIN_IN6_H */
