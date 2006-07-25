/* Original linux netdb.h merged with winsock.h types */

/*-
 * Copyright (c) 1980, 1983, 1988, 1993
 *     The Regents of the University of California.  All rights reserved.
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
 *	@(#)netdb.h	8.1 (Berkeley) 6/2/93
 *      netdb.h,v 1.1.1.1 1995/02/18 05:34:07 hjl Exp
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify and distribute this software for any
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

#ifndef _NETDB_H_
#define _NETDB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <cygwin/socket.h>

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */

  /* Different from the linux versions - note the shorts.. */
struct	hostent {
	const char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	short	h_addrtype;	/* host address type */
	short	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in an unsigned long -- probably a poor one.
 */

struct	netent {
	char		*n_name;	/* official name of net */
	char		**n_aliases;	/* alias list */
	short		n_addrtype;	/* net address type */
	uint32_t	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	short   s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent
{
  char	*p_name;	/* official protocol name */
  char	**p_aliases;	/* alias list */
  short	p_proto;	/* protocol # */
};

struct rpcent {
	char	*r_name;	/* name of server for this rpc program */
	char	**r_aliases;	/* alias list */
	int	r_number;	/* rpc program number */
};

struct addrinfo {
  int             ai_flags;		/* input flags */
  int             ai_family;		/* address family of socket */
  int             ai_socktype;		/* socket type */
  int             ai_protocol;		/* ai_protocol */
  socklen_t       ai_addrlen;		/* length of socket address */
  char            *ai_canonname;	/* canonical name of service location */
  struct sockaddr *ai_addr;		/* socket address of socket */
  struct addrinfo *ai_next;		/* pointer to next in list */
};

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#ifdef  __INSIDE_CYGWIN_NET__
extern int h_errno;
#else
extern __declspec(dllimport) int h_errno;
#endif

#define	NETDB_INTERNAL -1 /* see errno */
#define	NETDB_SUCCESS   0 /* no problem */
#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */

#define AI_PASSIVE      1
#define AI_CANONNAME    2
#define AI_NUMERICHOST  4
/*
 * These are not available in the WinSock implementation.  It wouldn't make
 * sense to support them in the ipv4 only case, so we drop them entirely.
 * We can define them if we run into problems but they are non-functional, so...
 */
#if 0
#define AI_V4MAPPED     16
#define AI_ALL          32
#define AI_ADDRCONFIG   64
#endif

#define NI_NOFQDN       1
#define NI_NUMERICHOST  2
#define NI_NAMEREQD     4
#define NI_NUMERICSERV  8
#define NI_DGRAM        16

#define NI_MAXHOST      1025
#define NI_MAXSERV      32

#define EAI_ADDRFAMILY  1
#define EAI_AGAIN       2
#define EAI_BADFLAGS    3
#define EAI_FAIL        4
#define EAI_FAMILY      5
#define EAI_MEMORY      6
#define EAI_NODATA      7
#define EAI_NONAME      8
#define EAI_SERVICE     9
#define EAI_SOCKTYPE    10
#define EAI_SYSTEM      11
#define EAI_BADHINTS    12
#define EAI_PROTOCOL    13

#define EAI_MAX		14

#ifndef __INSIDE_CYGWIN_NET__
void		endhostent (void);
void		endnetent (void);
void		endprotoent (void);
void		endservent (void);
void		endrpcent  (void);
struct hostent	*gethostbyaddr (const char *, int, int);
struct hostent	*gethostbyname (const char *);
struct hostent	*gethostent (void);
struct netent	*getnetbyaddr (long, int); /* u_long? */
struct netent	*getnetbyname (const char *);
struct netent	*getnetent (void);
struct protoent	*getprotobyname (const char *);
struct protoent	*getprotobynumber (int);
struct protoent	*getprotoent (void);
struct servent	*getservbyname (const char *, const char *);
struct servent	*getservbyport (int, const char *);
struct servent	*getservent (void);
struct rpcent	*getrpcent (void);
struct rpcent	*getrpcbyname (const char *);
struct rpcent	*getrpcbynumber (int);
const char      *hstrerror (int);
void		herror (const char *);
void		sethostent (int);
void		setnetent (int);
void		setprotoent (int);
void		setservent (int);
void		setrpcent (int);
void		freeaddrinfo (struct addrinfo *);
const char	*gai_strerror (int);
int		getaddrinfo (const char *, const char *,
			     const struct addrinfo *, struct addrinfo **);
int		getnameinfo (const struct sockaddr *, socklen_t, char *,
			     socklen_t, char *, socklen_t, int);

int		rcmd (char **, uint16_t, const char *, const char *,
		      const char *, int *);
int		rcmd_af (char **, uint16_t, const char *, const char *,
			 const char *, int *, int);
int		rexec (char **, uint16_t rport, char *, char *, char *, int *);
int		rresvport (int *);
int		rresvport_af (int *, int);
int		iruserok (unsigned long, int, const char *, const char *);
int		iruserok_sa (const void *, int, int, const char *,
			     const char *);
int		ruserok (const char *, int, const char *, const char *);
#endif

#ifdef __cplusplus
};
#endif

#endif /* !_NETDB_H_ */

