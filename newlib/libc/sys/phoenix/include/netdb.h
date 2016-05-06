/*-
 * Copyright (c) 1980, 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * -
 * --Copyright--
 */

/*
 *      @(#)netdb.h	8.1 (Berkeley) 6/2/93
 *      From: Id: netdb.h,v 8.9 1996/11/19 08:39:29 vixie Exp $
 * $FreeBSD: src/include/netdb.h,v 1.23 2002/03/23 17:24:53 imp Exp $
 */
 
/* Copied from Linux, modified for Phoenix-RTOS. */

#ifndef _NETDB_H
#define _NETDB_H

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifndef _PATH_HEQUIV
#define	_PATH_HEQUIV			"/etc/hosts.equiv"
#endif
#define	_PATH_HOSTS				"/etc/hosts"
#define	_PATH_NETWORKS			"/etc/networks"
#define	_PATH_PROTOCOLS			"/etc/protocols"
#define	_PATH_SERVICES			"/etc/services"
#define _PATH_NSSWITCH_CONF		"/etc/nsswitch.conf"

extern int *__h_errno_location(void);

#define h_errno (*(__h_errno_location()))

#define	MAXALIASES	35
/* For now, only support one return address. */
#define MAXADDRS         2

struct hostent {
	char *h_name;			/* Official name of host */
	char **h_aliases;		/* Alias list */
	int	h_addrtype;			/* Host address type */
	int	h_length;			/* Length of address */
	char **h_addr_list;		/* List of addresses from name server */
	char *h_addr;			/* Address, for backward compatibility */
	/* Private data, for re-entrancy */
	char *__host_addrs[MAXADDRS];
	char *__host_aliases[MAXALIASES];
	unsigned int __host_addr[4];
};

/* Assumption here is that a network number fits in an unsigned long -- probably a poor one. */
struct netent {
	char *n_name;			/* Official name of net */
	char **n_aliases;		/* Alias list */
	int	n_addrtype;			/* Net address type */
	unsigned long n_net;	/* Network # */
};

struct servent {
	char *s_name;			/* Official service name */
	char **s_aliases;		/* Alias list */
	int s_port;				/* Port # */
	char *s_proto;			/* Protocol to use */
};

struct protoent {
	char *p_name;			/* Official protocol name */
	char **p_aliases;		/* Alias list */
	int	p_proto;			/* Protocol # */
};

struct addrinfo {
	int	ai_flags;			/* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
	int	ai_family;			/* PF_xxx */
	int	ai_socktype;		/* SOCK_xxx */
	int	ai_protocol;		/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
	size_t	ai_addrlen;		/* Length of ai_addr */
	char *ai_canonname;		/* Canonical name for hostname */
	struct sockaddr *ai_addr;	/* Binary address */
	struct addrinfo *ai_next;	/* Next structure in linked list */
};

/* Error return codes from gethostbyname() and gethostbyaddr() (left in extern int h_errno). */
#define	NETDB_INTERNAL		-1		/* See errno */
#define	NETDB_SUCCESS		0		/* No problem */
#define	HOST_NOT_FOUND		1		/* Authoritative Answer Host not found */
#define	TRY_AGAIN			2		/* Non-Authoritative Host not found, or SERVERFAIL */
#define	NO_RECOVERY			3		/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA				4		/* Valid name, no data record of requested type */
#define	NO_ADDRESS			NO_DATA	/* No address, look for MX record */

/* Error return codes from getaddrinfo() */
#define EAI_BADFLAGS		-1		/* Invalid value for `ai_flags' field.  */
#define EAI_NONAME			-2		/* NAME or SERVICE is unknown.  */
#define EAI_AGAIN			-3		/* Temporary failure in name resolution.  */
#define EAI_FAIL			-4		/* Non-recoverable failure in name res.  */
#define EAI_NODATA			-5		/* No address associated with NAME.  */
#define EAI_FAMILY			-6		/* `ai_family' not supported.  */
#define EAI_SOCKTYPE		-7		/* `ai_socktype' not supported.  */
#define EAI_SERVICE			-8		/* SERVICE not supported for `ai_socktype'.  */
#define EAI_ADDRFAMILY		-9		/* Address family for NAME not supported.  */
#define EAI_MEMORY			-10		/* Memory allocation failure.  */
#define EAI_SYSTEM			-11		/* System error returned in `errno'.  */
#define EAI_OVERFLOW		-12		/* Argument buffer overflow.  */
#ifdef __USE_GNU
#define EAI_INPROGRESS		-100	/* Processing request in progress.  */
#define EAI_CANCELED		-101	/* Request canceled.  */
#define EAI_NOTCANCELED		-102	/* Request not canceled.  */
#define EAI_ALLDONE			-103	/* All requests done.  */
#define EAI_INTR			-104	/* Interrupted by a signal.  */
#define EAI_IDN_ENCODE		-105	/* IDN encoding failed.  */
#endif

/* Flag values for getaddrinfo() */
#define	AI_PASSIVE			0x00000001		/* Get address to use bind() */
#define	AI_CANONNAME		0x00000002		/* Fill ai_canonname */
#define	AI_NUMERICHOST		0x00000004		/* Prevent name resolution */
#define AI_NUMERICSERV		0x00000008		/* Fon't use name resolution. */

/* Valid flags for addrinfo */
#define AI_MASK				(AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_ADDRCONFIG)
#define	AI_ALL				0x00000100		/* IPv6 and IPv4-mapped (with AI_V4MAPPED) */
#define	AI_V4MAPPED_CFG		0x00000200		/* Accept IPv4-mapped if kernel supports */
#define	AI_ADDRCONFIG		0x00000400		/* Only if any address is assigned */
#define	AI_V4MAPPED			0x00000800		/* Accept IPv4-mapped IPv6 address */

/* Special recommended flags for getipnodebyname */
#define	AI_DEFAULT			(AI_V4MAPPED_CFG | AI_ADDRCONFIG)

/* Constants for getnameinfo() */
#define	NI_MAXHOST			1025
#define	NI_MAXSERV			32

/* Flag values for getnameinfo() */
#define	NI_NOFQDN			0x00000001
#define	NI_NUMERICHOST		0x00000002
#define	NI_NAMEREQD			0x00000004
#define	NI_NUMERICSERV		0x00000008
#define	NI_DGRAM			0x00000010
#define NI_WITHSCOPEID		0x00000020

/* Scope delimit character. */
#define	SCOPE_DELIMITER		'%'

void endhostent();
void endnetent();
void endnetgrent();
void endprotoent();
void endservent();
void freehostent(struct hostent *ip);
struct hostent *gethostbyaddr(const void *name, socklen_t len, int type);
struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyname2(const char *name, int af);
struct hostent *gethostent();
struct hostent *getipnodebyaddr(const void *addr, size_t len, int af, int *error_num);
struct hostent *getipnodebyname(const char *name, int af, int flags, int *error_num);
struct netent *getnetbyaddr(uint32_t net, int type);
struct netent *getnetbyname(const char *name);
struct netent *getnetent();
int getnetgrent(char **host, char **user, char **domain);
struct protoent	*getprotobyname(const char *name);
struct protoent	*getprotobynumber(int proto);
struct protoent	*getprotoent();
struct servent *getservbyname(const char *name, const char *proto);
struct servent *getservbyport(int name, const char *proto);
struct servent *getservent();
void herror(const char *s);
const char *hstrerror(int err);
int innetgr(const char *netgroup, const char *host, const char *user, const char *domain);
void sethostent(int stayopen);
void setnetent(int stayopen);
void setprotoent(int stayopen);
int	getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
int	getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);
void freeaddrinfo(struct addrinfo *res);
char *gai_strerror(int errcode);
int	setnetgrent(const char *netgroup);
void setservent(int stayopen);

#endif
