/*
 * Copyright (c) 1983, 1993, 1994
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
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rcmd.c	8.3 (Berkeley) 3/26/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
#ifndef __CYGWIN__
__FBSDID("$FreeBSD$");
#else
#define  __INSIDE_CYGWIN_NET__
#include "winsup.h"
#undef  __INSIDE_CYGWIN_NET__
#endif

#ifndef __CYGWIN__
#include "namespace.h"
#endif
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifndef __CYGWIN__
#include <rpc/rpc.h>
#endif
#ifdef YP
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#endif
#ifndef __CYGWIN__
#include <arpa/nameser.h>
#include "un-namespace.h"
#endif

#ifndef __CYGWIN__
extern int innetgr( const char *, const char *, const char *, const char * );

#define max(a, b)	((a > b) ? a : b)
#else
#include "wininet.h"
#include "cygwin/in6.h"

#ifndef _PATH_HEQUIV
# define _PATH_HEQUIV "/etc/hosts.equiv"
#endif

#define innetgr(a,b,c,d)	(0)

extern int rcmdsh(char **, int, const char *, const char *, const char *,
		  const char *);

extern "C" {
  int cygwin_accept (int, struct sockaddr *, socklen_t *);
  int cygwin_bindresvport_sa (int, struct sockaddr *);
  int cygwin_connect (int, const struct sockaddr *, socklen_t);
  void cygwin_freeaddrinfo (struct addrinfo *);
  const char * cygwin_gai_strerror (int);
  int cygwin_getaddrinfo (const char *, const char *, const struct addrinfo *,
			  struct addrinfo **);
  int cygwin_getnameinfo (const struct sockaddr *, socklen_t, char *, size_t,
  			  char *, size_t, int);
  struct servent *cygwin_getservbyname (const char *, const char *);
  int cygwin_listen (int, int);
  int cygwin_rresvport_af(int *alport, int family);
  int cygwin_select (int, fd_set *, fd_set *, fd_set *, struct timeval *);
  int cygwin_socket (int, int, int);
  int seteuid32 (uid_t);
}
#endif

#ifndef __CYGWIN__
static int __ivaliduser(FILE *, u_int32_t, const char *, const char *);
static int __ivaliduser_af(FILE *,const void *, const char *, const char *,
			   int, int);
#endif
static int __ivaliduser_sa(FILE *, const struct sockaddr *, socklen_t,
			   const char *, const char *);
static int __icheckhost(const struct sockaddr *, socklen_t, const char *);

char paddr[NI_MAXHOST];

extern "C" int
cygwin_rcmd_af(char **ahost, in_port_t rport, const char *locuser,
	       const char *remuser, const char *cmd, int *fd2p, int af)
{
	struct addrinfo hints, *res, *ai;
	struct sockaddr_storage from;
	fd_set reads;
	sigset_t oldmask, newmask;
	pid_t pid;
	int s, aport, lport, timo, error;
	char c;//, *p;
	int refused, nres;
	char num[8];
	static char canonnamebuf[INTERNET_MAX_HOST_NAME_LENGTH + 1];	/* is it proper here? */
#ifndef __CYGWIN__
	/* call rcmdsh() with specified remote shell if appropriate. */
	if (!issetugid() && (p = getenv("RSH"))) {
		struct servent *sp = cygwin_getservbyname("shell", "tcp");

		if (sp && sp->s_port == rport)
			return (rcmdsh(ahost, rport, locuser, remuser,
			    cmd, p));
	}

	/* use rsh(1) if non-root and remote port is shell. */
	if (geteuid()) {
		struct servent *sp = cygwin_getservbyname("shell", "tcp");

		if (sp && sp->s_port == rport)
			return (rcmdsh(ahost, rport, locuser, remuser,
			    cmd, NULL));
	}
#endif
	pid = getpid();

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = af;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	(void)snprintf(num, sizeof(num), "%d", ntohs(rport));
	error = cygwin_getaddrinfo(*ahost, num, &hints, &res);
	if (error) {
		fprintf(stderr, "rcmd: getaddrinfo: %s\n",
			cygwin_gai_strerror(error));
		if (error == EAI_SYSTEM)
			fprintf(stderr, "rcmd: getaddrinfo: %s\n",
				strerror(errno));
		return (-1);
	}

	if (res->ai_canonname
	 && strlen(res->ai_canonname) + 1 < sizeof(canonnamebuf)) {
		strncpy(canonnamebuf, res->ai_canonname, sizeof(canonnamebuf));
		*ahost = canonnamebuf;
	}
	nres = 0;
	for (ai = res; ai; ai = ai->ai_next)
		nres++;
	ai = res;
	refused = 0;
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGURG);
	sigprocmask(SIG_BLOCK, (const sigset_t *)&newmask, &oldmask);
	for (timo = 1, lport = IPPORT_RESERVED - 1;;) {
		s = cygwin_rresvport_af(&lport, ai->ai_family);
		if (s < 0) {
			if (errno != EAGAIN && ai->ai_next) {
				ai = ai->ai_next;
				continue;
			}
			if (errno == EAGAIN)
				(void)fprintf(stderr,
				    "rcmd: socket: All ports in use\n");
			else
				(void)fprintf(stderr, "rcmd: socket: %s\n",
				    strerror(errno));
			cygwin_freeaddrinfo(res);
			sigprocmask(SIG_SETMASK, (const sigset_t *)&oldmask,
			    NULL);
			return (-1);
		}
		fcntl64(s, F_SETOWN, pid);
		if (cygwin_connect(s, ai->ai_addr, ai->ai_addrlen) >= 0)
			break;
		(void)close(s);
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
		if (errno == ECONNREFUSED)
			refused = 1;
		if (ai->ai_next == NULL && (!refused || timo > 16)) {
			(void)fprintf(stderr, "%s: %s\n",
				      *ahost, strerror(errno));
			cygwin_freeaddrinfo(res);
			sigprocmask(SIG_SETMASK, (const sigset_t *)&oldmask,
			    NULL);
			return (-1);
		}
		if (nres > 1) {
			int oerrno = errno;

			cygwin_getnameinfo(ai->ai_addr, ai->ai_addrlen, paddr,
			    sizeof(paddr), NULL, 0, NI_NUMERICHOST);
			(void)fprintf(stderr, "connect to address %s: ",
				      paddr);
			errno = oerrno;
			perror(0);
		}
		if ((ai = ai->ai_next) == NULL) {
			/* refused && timo <= 16 */
			struct timespec time_to_sleep, time_remaining;

			time_to_sleep.tv_sec = timo;
			time_to_sleep.tv_nsec = 0;
			(void)nanosleep(&time_to_sleep, &time_remaining);
			timo *= 2;
			ai = res;
			refused = 0;
		}
		if (nres > 1) {
			cygwin_getnameinfo(ai->ai_addr, ai->ai_addrlen, paddr,
			    sizeof(paddr), NULL, 0, NI_NUMERICHOST);
			fprintf(stderr, "Trying %s...\n", paddr);
		}
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		int s2 = cygwin_rresvport_af(&lport, ai->ai_family), s3;
		socklen_t len = ai->ai_addrlen;
		int nfds;

		if (s2 < 0)
			goto bad;
		cygwin_listen(s2, 1);
		(void)snprintf(num, sizeof(num), "%d", lport);
		if (write(s, num, strlen(num)+1) != (int)strlen(num)+1) {
			(void)fprintf(stderr,
			    "rcmd: write (setting up stderr): %s\n",
			    strerror(errno));
			(void)close(s2);
			goto bad;
		}
		nfds = max(s, s2)+1;
		if(nfds > FD_SETSIZE) {
			fprintf(stderr, "rcmd: too many files\n");
			(void)close(s2);
			goto bad;
		}
again:
		FD_ZERO(&reads);
		FD_SET(s, &reads);
		FD_SET(s2, &reads);
		errno = 0;
		if (cygwin_select(nfds, &reads, 0, 0, 0) < 1 || !FD_ISSET(s2, &reads)){
			if (errno != 0)
				(void)fprintf(stderr,
				    "rcmd: select (setting up stderr): %s\n",
				    strerror(errno));
			else
				(void)fprintf(stderr,
				"select: protocol failure in circuit setup\n");
			(void)close(s2);
			goto bad;
		}
		s3 = cygwin_accept(s2, (struct sockaddr *)&from, &len);
		switch (from.ss_family) {
		case AF_INET:
			aport = ntohs(((struct sockaddr_in *)&from)->sin_port);
			break;
		case AF_INET6:
			aport = ntohs(((struct sockaddr_in6 *)&from)->sin6_port);
			break;
		default:
			aport = 0;	/* error */
			break;
		}
		/*
		 * XXX careful for ftp bounce attacks. If discovered, shut them
		 * down and check for the real auxiliary channel to connect.
		 */
		if (aport == 20) {
			close(s3);
			goto again;
		}
		(void)close(s2);
		if (s3 < 0) {
			(void)fprintf(stderr,
			    "rcmd: accept: %s\n", strerror(errno));
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		if (aport >= IPPORT_RESERVED || aport < IPPORT_RESERVED / 2) {
			(void)fprintf(stderr,
			    "socket: protocol failure in circuit setup.\n");
			goto bad2;
		}
	}
	(void)write(s, locuser, strlen(locuser)+1);
	(void)write(s, remuser, strlen(remuser)+1);
	(void)write(s, cmd, strlen(cmd)+1);
	if (read(s, &c, 1) != 1) {
		(void)fprintf(stderr,
		    "rcmd: %s: %s\n", *ahost, strerror(errno));
		goto bad2;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void)write(STDERR_FILENO, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
	sigprocmask(SIG_SETMASK, (const sigset_t *)&oldmask, NULL);
	cygwin_freeaddrinfo(res);
	return (s);
bad2:
	if (lport)
		(void)close(*fd2p);
bad:
	(void)close(s);
	sigprocmask(SIG_SETMASK, (const sigset_t *)&oldmask, NULL);
	cygwin_freeaddrinfo(res);
	return (-1);
}

extern "C" int
cygwin_rcmd(char **ahost, in_port_t rport, const char *locuser,
	    const char *remuser, const char *cmd, int *fd2p)
{
	return cygwin_rcmd_af(ahost, rport, locuser, remuser, cmd, fd2p,
			      AF_INET);
}

extern "C" int
cygwin_rresvport_af(int *alport, int family)
{
	int s;
	struct sockaddr_storage ss;
	u_short *sport;

	memset(&ss, 0, sizeof(ss));
	ss.ss_family = family;
	switch (family) {
	case AF_INET:
		sport = &((struct sockaddr_in *)&ss)->sin_port;
		((struct sockaddr_in *)&ss)->sin_addr.s_addr = INADDR_ANY;
		break;
	case AF_INET6:
		sport = &((struct sockaddr_in6 *)&ss)->sin6_port;
		((struct sockaddr_in6 *)&ss)->sin6_addr = in6addr_any;
		break;
	default:
		errno = EAFNOSUPPORT;
		return -1;
	}

	s = cygwin_socket(ss.ss_family, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);
#if 0 /* compat_exact_traditional_rresvport_semantics */
	sin.sin_port = htons((u_short)*alport);
	if (_bind(s, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
		return (s);
	if (errno != EADDRINUSE) {
		(void)_close(s);
		return (-1);
	}
#endif
	*sport = 0;
	if (cygwin_bindresvport_sa(s, (struct sockaddr *)&ss) == -1) {
		(void)close(s);
		return (-1);
	}
	*alport = (int)ntohs(*sport);
	return (s);
}

extern "C" int
cygwin_rresvport(int *port)
{
	return cygwin_rresvport_af(port, AF_INET);
}

int	__check_rhosts_file = 1;
char	*__rcmd_errstr;

/*
 * AF independent extension of iruserok.
 *
 * Returns 0 if ok, -1 if not ok.
 */
extern "C" int
iruserok_sa(const void *ra, int rlen, int superuser, const char *ruser,
	    const char *luser)
{
	const char *cp;
	struct stat sbuf;
	struct passwd *pwd;
	FILE *hostf;
	uid_t uid;
	int first;
	char pbuf[MAXPATHLEN];
	const struct sockaddr *raddr;
	struct sockaddr_storage ss;

	/* avoid alignment issue */
	if (rlen > (int) sizeof(ss))
		return(-1);
	memcpy(&ss, ra, rlen);
	raddr = (struct sockaddr *)&ss;

	first = 1;
	hostf = superuser ? NULL : fopen(_PATH_HEQUIV, "rt");
again:
	if (hostf) {
		if (__ivaliduser_sa(hostf, raddr, rlen, luser, ruser) == 0) {
			(void)fclose(hostf);
			return (0);
		}
		(void)fclose(hostf);
	}
	if (first == 1 && (__check_rhosts_file || superuser)) {
		first = 0;
		if ((pwd = getpwnam(luser)) == NULL)
			return (-1);
		(void)strcpy(pbuf, pwd->pw_dir);
		(void)strcat(pbuf, "/.rhosts");

		/*
		 * Change effective uid while opening .rhosts.  If root and
		 * reading an NFS mounted file system, can't read files that
		 * are protected read/write owner only.
		 */
		uid = geteuid32();
		(void)seteuid32(pwd->pw_uid);
		hostf = fopen(pbuf, "rt");
		(void)seteuid32(uid);

		if (hostf == NULL)
			return (-1);
		/*
		 * If not a regular file, or is owned by someone other than
		 * user or root or if writeable by anyone but the owner, quit.
		 */
		cp = NULL;
		if (lstat64(pbuf, &sbuf) < 0)
			cp = ".rhosts lstat failed";
		else if (!S_ISREG(sbuf.st_mode))
			cp = ".rhosts not regular file";
		else if (fstat64(fileno(hostf), &sbuf) < 0)
			cp = ".rhosts fstat failed";
		else if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid)
			cp = "bad .rhosts owner";
		else if (sbuf.st_mode & (S_IWGRP|S_IWOTH))
			cp = ".rhosts writeable by other than owner";
		/* If there were any problems, quit. */
		if (cp) {
			__rcmd_errstr = (char *) cp;
			(void)fclose(hostf);
			return (-1);
		}
		goto again;
	}
	return (-1);
}

/*
 * New .rhosts strategy: We are passed an ip address. We spin through
 * hosts.equiv and .rhosts looking for a match. When the .rhosts only
 * has ip addresses, we don't have to trust a nameserver.  When it
 * contains hostnames, we spin through the list of addresses the nameserver
 * gives us and look for a match.
 *
 * Returns 0 if ok, -1 if not ok.
 */
extern "C" int
iruserok(unsigned long raddr, int superuser, const char *ruser,
	 const char *luser)
{
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	memcpy(&sin.sin_addr, &raddr, sizeof(sin.sin_addr));
	return iruserok_sa((struct sockaddr *)&sin, sizeof(struct sockaddr_in),
			   superuser, ruser, luser);
}

extern "C" int
ruserok(const char *rhost, int superuser, const char *ruser, const char *luser)
{
	struct addrinfo hints, *res, *r;
	int error;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;	/*dummy*/
	error = cygwin_getaddrinfo(rhost, "0", &hints, &res);
	if (error)
		return (-1);

	for (r = res; r; r = r->ai_next) {
		if (iruserok_sa(r->ai_addr, r->ai_addrlen, superuser, ruser,
		    luser) == 0) {
			cygwin_freeaddrinfo(res);
			return (0);
		}
	}
	cygwin_freeaddrinfo(res);
	return (-1);
}

#ifndef __CYGWIN__
/*
 * XXX
 * Don't make static, used by lpd(8).
 *
 * Returns 0 if ok, -1 if not ok.
 */
static int
__ivaliduser(FILE *hostf, u_int32_t raddr, const char *luser, const char *ruser)
{
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	memcpy(&sin.sin_addr, &raddr, sizeof(sin.sin_addr));
	return __ivaliduser_sa(hostf, (struct sockaddr *)&sin,
			       sizeof(struct sockaddr_in), luser, ruser);
}

/*
 * Returns 0 if ok, -1 if not ok.
 *
 * XXX obsolete API.
 */
static int
__ivaliduser_af(FILE *hostf, const void *raddr, const char *luser,
		const char *ruser, int af, int len)
{
	struct sockaddr *sa = NULL;
	struct sockaddr_in *sin = NULL;
#ifdef INET6
	struct sockaddr_in6 *sin6 = NULL;
#endif
	struct sockaddr_storage ss;
	int salen = 0;

	memset(&ss, 0, sizeof(ss));
	switch (af) {
	case AF_INET:
		if (len != sizeof(sin->sin_addr))
			return -1;
		sin = (struct sockaddr_in *)&ss;
		sin->sin_family = AF_INET;
		salen = sizeof(struct sockaddr_in);
		memcpy(&sin->sin_addr, raddr, sizeof(sin->sin_addr));
		break;
	case AF_INET6:
		if (len != sizeof(sin6->sin6_addr))
			return -1;
		/* you will lose scope info */
		sin6 = (struct sockaddr_in6 *)&ss;
		sin6->sin6_family = AF_INET6;
		salen = sizeof(struct sockaddr_in6);
		memcpy(&sin6->sin6_addr, raddr, sizeof(sin6->sin6_addr));
		break;
	default:
		return -1;
	}

	sa = (struct sockaddr *)&ss;
	return __ivaliduser_sa(hostf, sa, salen, luser, ruser);
}
#endif

static int
__ivaliduser_sa(FILE *hostf, const struct sockaddr *raddr, socklen_t salen,
		const char *luser, const char *ruser)
{
	char *user, *p;
	int ch;
	char buf[MAXHOSTNAMELEN + 128];		/* host + login */
	char hname[MAXHOSTNAMELEN];
	/* Presumed guilty until proven innocent. */
	int userok = 0, hostok = 0;
#ifdef YP
	char *ypdomain;

	if (yp_get_default_domain(&ypdomain))
		ypdomain = NULL;
#else
#define	ypdomain NULL
#endif
	/* We need to get the damn hostname back for netgroup matching. */
	if (cygwin_getnameinfo(raddr, salen, hname, sizeof(hname), NULL, 0,
			NI_NAMEREQD) != 0)
		hname[0] = '\0';

	while (fgets(buf, sizeof(buf), hostf)) {
		p = buf;
		/* Skip lines that are too long. */
		if (strchr(p, '\n') == NULL) {
			while ((ch = getc(hostf)) != '\n' && ch != EOF);
			continue;
		}
		if (*p == '\n' || *p == '#') {
			/* comment... */
			continue;
		}
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') {
			*p = isupper((unsigned char)*p) ? tolower((unsigned char)*p) : *p;
			p++;
		}
		if (*p == ' ' || *p == '\t') {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
			user = p;
			while (*p != '\n' && *p != ' ' &&
			    *p != '\t' && *p != '\0')
				p++;
		} else
			user = p;
		*p = '\0';
		/*
		 * Do +/- and +@/-@ checking. This looks really nasty,
		 * but it matches SunOS's behavior so far as I can tell.
		 */
		switch(buf[0]) {
		case '+':
			if (!buf[1]) {     /* '+' matches all hosts */
				hostok = 1;
				break;
			}
			if (buf[1] == '@')  /* match a host by netgroup */
				hostok = hname[0] != '\0' &&
				    innetgr(&buf[2], hname, NULL, ypdomain);
			else		/* match a host by addr */
				hostok = __icheckhost(raddr, salen,
						      (char *)&buf[1]);
			break;
		case '-':     /* reject '-' hosts and all their users */
			if (buf[1] == '@') {
				if (hname[0] == '\0' ||
				    innetgr(&buf[2], hname, NULL, ypdomain))
					return(-1);
			} else {
				if (__icheckhost(raddr, salen,
						 (char *)&buf[1]))
					return(-1);
			}
			break;
		default:  /* if no '+' or '-', do a simple match */
			hostok = __icheckhost(raddr, salen, buf);
			break;
		}
		switch(*user) {
		case '+':
			if (!*(user+1)) {      /* '+' matches all users */
				userok = 1;
				break;
			}
			if (*(user+1) == '@')  /* match a user by netgroup */
				userok = innetgr(user+2, NULL, ruser, ypdomain);
			else	   /* match a user by direct specification */
				userok = !(strcmp(ruser, user+1));
			break;
		case '-': 		/* if we matched a hostname, */
			if (hostok) {   /* check for user field rejections */
				if (!*(user+1))
					return(-1);
				if (*(user+1) == '@') {
					if (innetgr(user+2, NULL,
							ruser, ypdomain))
						return(-1);
				} else {
					if (!strcmp(ruser, user+1))
						return(-1);
				}
			}
			break;
		default:	/* no rejections: try to match the user */
			if (hostok)
				userok = !(strcmp(ruser,*user ? user : luser));
			break;
		}
		if (hostok && userok)
			return(0);
	}
	return (-1);
}

/*
 * Returns "true" if match, 0 if no match.
 */
static int
__icheckhost(const struct sockaddr *raddr, socklen_t salen, const char *lhost)
{
	struct sockaddr_in sin;
	struct sockaddr_in6 *sin6;
	struct addrinfo hints, *res, *r;
	int error;
	char h1[NI_MAXHOST], h2[NI_MAXHOST];

	if (raddr->sa_family == AF_INET6) {
		sin6 = (struct sockaddr_in6 *)raddr;
		if (IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr)) {
			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			memcpy(&sin.sin_addr, &sin6->sin6_addr.s6_addr[12],
			       sizeof(sin.sin_addr));
			raddr = (struct sockaddr *)&sin;
			salen = sizeof(struct sockaddr_in);
		}
	}

	h1[0] = '\0';
	if (cygwin_getnameinfo(raddr, salen, h1, sizeof(h1), NULL, 0,
			NI_NUMERICHOST) != 0)
		return (0);

	/* Resolve laddr into sockaddr */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = raddr->sa_family;
	hints.ai_socktype = SOCK_DGRAM;	/*XXX dummy*/
	res = NULL;
	error = cygwin_getaddrinfo(lhost, "0", &hints, &res);
	if (error)
		return (0);

	for (r = res; r ; r = r->ai_next) {
		h2[0] = '\0';
		if (cygwin_getnameinfo(r->ai_addr, r->ai_addrlen, h2, sizeof(h2),
				NULL, 0, NI_NUMERICHOST) != 0)
			continue;
		if (strcmp(h1, h2) == 0) {
			cygwin_freeaddrinfo(res);
			return (1);
		}
	}

	/* No match. */
	cygwin_freeaddrinfo(res);
	return (0);
}
