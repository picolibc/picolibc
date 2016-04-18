/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions of the Internet Protocol.
 *
 * Version:	@(#)in.h	1.0.1	04/21/93
 *
 * Authors:	Original taken from the GNU Project <netinet/in.h> file.
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _CYGWIN_IN_H
#define _CYGWIN_IN_H

#include <cygwin/socket.h>

#ifndef _IN_ADDR_T_DECLARED
typedef	__uint32_t	in_addr_t;
#define	_IN_ADDR_T_DECLARED
#endif

#ifndef _IN_PORT_T_DECLARED
typedef	__uint16_t	in_port_t;
#define	_IN_PORT_T_DECLARED
#endif

#ifndef __INSIDE_CYGWIN_NET__

/* Standard well-defined IP protocols.  If you ever add one here, don't
   forget to define it below. */
enum
{
  IPPROTO_IP = 0,		/* Dummy protocol for TCP		*/
  IPPROTO_HOPOPTS = 0,		/* IPv6 Hop-by-Hop options		*/
  IPPROTO_ICMP = 1,		/* Internet Control Message Protocol	*/
  IPPROTO_IGMP = 2,		/* Internet Gateway Management Protocol */
  IPPROTO_IPIP = 4,		/* IPIP tunnels (older KA9Q tunnels use 94) */
  IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
  IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
  IPPROTO_PUP = 12,		/* PUP protocol				*/
  IPPROTO_UDP = 17,		/* User Datagram Protocol		*/
  IPPROTO_IDP = 22,		/* XNS IDP protocol			*/
  IPPROTO_IPV6 = 41,		/* IPv6 header				*/
  IPPROTO_ROUTING = 43,		/* IPv6 Routing header			*/
  IPPROTO_FRAGMENT = 44,	/* IPv6 fragmentation header		*/
  IPPROTO_ESP = 50,		/* encapsulating security payload	*/
  IPPROTO_AH = 51,		/* authentication header		*/
  IPPROTO_ICMPV6 = 58,		/* ICMPv6				*/
  IPPROTO_NONE = 59,		/* IPv6 no next header			*/
  IPPROTO_DSTOPTS = 60,		/* IPv6 Destination options		*/
  IPPROTO_RAW = 255,		/* Raw IP packets			*/
  IPPROTO_MAX
};

/* Define IPPROTO_xxx values to accomodate SUSv3 */
#define IPPROTO_IP IPPROTO_IP
#define IPPROTO_HOPOPTS IPPROTO_HOPOPTS
#define IPPROTO_ICMP IPPROTO_ICMP
#define IPPROTO_IGMP IPPROTO_IGMP
#define IPPROTO_IPIP IPPROTO_IPIP
#define IPPROTO_TCP IPPROTO_TCP
#define IPPROTO_EGP IPPROTO_EGP
#define IPPROTO_PUP IPPROTO_PUP
#define IPPROTO_UDP IPPROTO_UDP
#define IPPROTO_IDP IPPROTO_IDP
#define IPPROTO_RAW IPPROTO_RAW
#define IPPROTO_IPV6 IPPROTO_IPV6
#define IPPROTO_ROUTING IPPROTO_ROUTING
#define IPPROTO_FRAGMENT IPPROTO_FRAGMENT
#define IPPROTO_ESP IPPROTO_ESP
#define IPPROTO_AH IPPROTO_AH
#define IPPROTO_ICMPV6 IPPROTO_ICMPV6
#define IPPROTO_NONE IPPROTO_NONE
#define IPPROTO_DSTOPTS IPPROTO_DSTOPTS

/* Standard well-known ports.  *//* from winsup/include/netinet/in.h */
enum
{
  IPPORT_ECHO = 7,		/* Echo service.  */
  IPPORT_DISCARD = 9,		/* Discard transmissions service.  */
  IPPORT_SYSTAT = 11,		/* System status service.  */
  IPPORT_DAYTIME = 13,	/* Time of day service.  */
  IPPORT_NETSTAT = 15,	/* Network status service.  */
  IPPORT_FTP = 21,		/* File Transfer Protocol.  */
  IPPORT_TELNET = 23,		/* Telnet protocol.  */
  IPPORT_SMTP = 25,		/* Simple Mail Transfer Protocol.  */
  IPPORT_TIMESERVER = 37,	/* Timeserver service.  */
  IPPORT_NAMESERVER = 42,	/* Domain Name Service.  */
  IPPORT_WHOIS = 43,		/* Internet Whois service.  */
  IPPORT_MTP = 57,

  IPPORT_TFTP = 69,		/* Trivial File Transfer Protocol.  */
  IPPORT_RJE = 77,
  IPPORT_FINGER = 79,		/* Finger service.  */
  IPPORT_TTYLINK = 87,
  IPPORT_SUPDUP = 95,		/* SUPDUP protocol.  */


  IPPORT_EXECSERVER = 512,	/* execd service.  */
  IPPORT_LOGINSERVER = 513,	/* rlogind service.  */
  IPPORT_CMDSERVER = 514,
  IPPORT_EFSSERVER = 520,

  /* UDP ports.  */
  IPPORT_BIFFUDP = 512,
  IPPORT_WHOSERVER = 513,
  IPPORT_ROUTESERVER = 520,

  /* Ports less than this value are reserved for privileged processes.  */
  IPPORT_RESERVED = 1024,

  /* Ports greater this value are reserved for (non-privileged) servers.  */
  IPPORT_USERRESERVED = 5000
};

/* Avoid collision with Mingw64 headers. */
#ifndef s_addr
/* Internet address. */
struct in_addr
{
  in_addr_t s_addr;
};
#define s_addr s_addr
#endif

/* Request struct for IPv4 multicast socket ops */

struct ip_mreq
{
  struct in_addr imr_multiaddr;	/* IP multicast address of group */
  struct in_addr imr_interface;	/* local IP address of interface */
};

struct ip_mreq_source
{
  struct in_addr imr_multiaddr;
  struct in_addr imr_sourceaddr;
  struct in_addr imr_interface;
};

struct ip_msfilter
{
  struct in_addr imsf_multiaddr;
  struct in_addr imsf_interface;
  uint32_t       imsf_fmode;
  uint32_t       imsf_numsrc;
  struct in_addr imsf_slist[1];
};

#define IP_MSFILTER_SIZE(numsrc) (sizeof (struct ip_msfilter) \
				  - sizeof (struct in_addr) \
				  + (numsrc) * sizeof (struct in_addr))

struct in_pktinfo
{
  struct in_addr ipi_addr;
  uint32_t       ipi_ifindex;
};

/* Request struct for IP agnostic multicast socket ops */

struct group_req
{
  uint32_t                gr_interface;
  struct sockaddr_storage gr_group;
};

struct group_source_req
{
  uint32_t                gsr_interface;
  struct sockaddr_storage gsr_group;
  struct sockaddr_storage gsr_source;
};

struct group_filter
{
  uint32_t                gf_interface;
  struct sockaddr_storage gf_group;
  uint32_t                gf_fmode;
  uint32_t                gf_numsrc;
  struct sockaddr_storage gf_slist[1];
};

#define GROUP_FILTER_SIZE(numsrc) (sizeof (struct group_filter) \
				   - sizeof (struct sockaddr_storage) \
				   + (numsrc) * sizeof (struct sockaddr_storage))

/* Structure describing an Internet (IP) socket address. */
#define __SOCK_SIZE__	16		/* sizeof(struct sockaddr)	*/
struct sockaddr_in
{
  sa_family_t	 sin_family;	/* Address family		*/
  in_port_t	 sin_port;	/* Port number			*/
  struct in_addr sin_addr;	/* Internet address		*/

  /* Pad to size of `struct sockaddr'. */
  unsigned char  __pad[__SOCK_SIZE__ - sizeof(short int)
			- sizeof(unsigned short int) - sizeof(struct in_addr)];
};
#define sin_zero	__pad		/* for BSD UNIX comp. -FvK	*/

/*
 * Definitions of the bits in an Internet address integer.
 * On subnets, host and network parts are found according
 * to the subnet mask, not these masks.
 */
#define	IN_CLASSA(a)		((((in_addr_t) (a)) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		(0xffffffff & ~IN_CLASSA_NET)
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(a)		((((in_addr_t) (a)) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		(0xffffffff & ~IN_CLASSB_NET)
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(a)		((((in_addr_t) (a)) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		(0xffffffff & ~IN_CLASSC_NET)

#define	IN_CLASSD(a)		((((in_addr_t) (a)) & 0xf0000000) == 0xe0000000)
#define	IN_MULTICAST(a)		IN_CLASSD(a)
#define IN_MULTICAST_NET	0xF0000000

#define	IN_EXPERIMENTAL(a)	((((in_addr_t) (a)) & 0xe0000000) == 0xe0000000)
#define	IN_BADCLASS(a)		((((in_addr_t) (a)) & 0xf0000000) == 0xf0000000)

/* Address to accept any incoming messages. */
#define	INADDR_ANY		((in_addr_t) 0x00000000)

/* Address to send to all hosts. */
#define	INADDR_BROADCAST	((in_addr_t) 0xffffffff)

/* Address indicating an error return. */
#define	INADDR_NONE		0xffffffff

/* Network number for local host loopback. */
#define	IN_LOOPBACKNET		127

/* Address to loopback in software to local host.  */
#define	INADDR_LOOPBACK		0x7f000001	/* 127.0.0.1   */
#define	IN_LOOPBACK(a)		((((in_addr_t) (a)) & 0xff000000) == 0x7f000000)

/* Defines for Multicast INADDR */
#define INADDR_UNSPEC_GROUP	0xe0000000      /* 224.0.0.0   */
#define INADDR_ALLHOSTS_GROUP	0xe0000001      /* 224.0.0.1   */
#define INADDR_ALLRTRS_GROUP	0xe0000002	/* 224.0.0.2   */
#define INADDR_MAX_LOCAL_GROUP  0xe00000ff      /* 224.0.0.255 */

#define INET_ADDRSTRLEN 16

/* <asm/byteorder.h> contains the htonl type stuff.. */

#include <asm/byteorder.h>

/* Some random defines to make it easier in the kernel.. */
#ifdef __KERNEL__

#define LOOPBACK(x)	(((x) & htonl(0xff000000)) == htonl(0x7f000000))
#define MULTICAST(x)	(((x) & htonl(0xf0000000)) == htonl(0xe0000000))

#endif

#ifdef AF_INET6
#include <cygwin/in6.h>
#endif
#endif

#endif	/* _CYGWIN_IN_H */
