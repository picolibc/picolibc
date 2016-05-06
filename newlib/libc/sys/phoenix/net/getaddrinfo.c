/*
 * Mar  8, 2000 by Hajimu UMEMOTO <ume@mahoroba.org>
 * $Id: getaddrinfo.c,v 1.2 2002/05/22 17:56:56 snsimon Exp $
 *
 * This module is besed on ssh-1.2.27-IPv6-1.5 written by
 * KIKUCHI Takahiro <kick@kyoto.wide.ad.jp>
 */
/*
 * Copyright (c) 2001 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any other legal
 *    details, please contact
 *      Office of Technology Transfer
 *      Carnegie Mellon University
 *      5000 Forbes Avenue
 *      Pittsburgh, PA  15213-3890
 *      (412) 268-4387, fax: (412) 268-7395
 *      tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * fake library for ssh
 *
 * This file includes getaddrinfo(), freeaddrinfo() and gai_strerror().
 * These funtions are defined in rfc2133.
 *
 * But these functions are not implemented correctly. The minimum subset
 * is implemented for ssh use only. For example, this routine assumes
 * that ai_family is AF_INET. Don't use it for another purpose.
 *
 * In the case not using 'configure --enable-ipv6', this getaddrinfo.c
 * will be used if you have broken getaddrinfo or no getaddrinfo.
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

static struct addrinfo *malloc_ai(int port, u_long addr, int socktype, int proto)
{
	struct addrinfo *ai = (struct addrinfo *)malloc(sizeof(struct addrinfo) + sizeof(struct sockaddr_in));
	if (ai) {
		memset(ai, 0, sizeof(struct addrinfo) + sizeof(struct sockaddr_in));
		ai->ai_addr = (struct sockaddr *)(ai + 1);
		/* XXX -- ssh doesn't use sa_len */
		ai->ai_addrlen = sizeof(struct sockaddr_in);
#ifdef HAVE_SOCKADDR_SA_LEN
		ai->ai_addr->sa_len = sizeof(struct sockaddr_in);
#endif
		ai->ai_addr->sa_family = ai->ai_family = AF_INET;
		((struct sockaddr_in *)(ai)->ai_addr)->sin_port = port;
		((struct sockaddr_in *)(ai)->ai_addr)->sin_addr.s_addr = addr;
		ai->ai_socktype = socktype;
		ai->ai_protocol = proto;
		return ai;
	}

	return NULL;
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
	struct addrinfo *cur, *prev = NULL;
	struct hostent *hp;
	struct in_addr in;
	int i, port = 0, socktype, proto;

	if (hints && hints->ai_family != PF_INET && hints->ai_family != PF_UNSPEC)
		return EAI_FAMILY;

	socktype = (hints && hints->ai_socktype) ? hints->ai_socktype : SOCK_STREAM;
	if (hints && hints->ai_protocol)
		proto = hints->ai_protocol;
	else {
		switch (socktype) {
		case SOCK_DGRAM:
			proto = IPPROTO_UDP;
			break;
		case SOCK_STREAM:
			proto = IPPROTO_TCP;
			break;
		default:
			proto = 0;
			break;
		}
	}

	if (service) {
		if (isdigit((int)*service))
			port = htons(atoi(service));
		else {
			struct servent *se;
			char *pe_proto;

			switch (socktype) {
			case SOCK_DGRAM:
				pe_proto = "udp";
				break;
			case SOCK_STREAM:
				pe_proto = "tcp";
				break;
			default:
				pe_proto = NULL;
				break;
			}
			if ((se = getservbyname(service, pe_proto)) == NULL)
				return EAI_SERVICE;

			port = se->s_port;
		}
	}
	if (!node) {
		if (hints && hints->ai_flags & AI_PASSIVE)
			*res = malloc_ai(port, htonl(0x00000000), socktype, proto);
		else
			*res = malloc_ai(port, htonl(0x7f000001), socktype, proto);
		if (*res)
			return 0;
		else
			return EAI_MEMORY;
	}
	if (inet_aton(node, &in)) {
		*res = malloc_ai(port, in.s_addr, socktype, proto);
		if (*res)
			return 0;
		else
			return EAI_MEMORY;
	}
	if (hints && hints->ai_flags & AI_NUMERICHOST)
		return EAI_NODATA;
	if ((hp = gethostbyname(node)) && hp->h_name && hp->h_name[0] && hp->h_addr_list[0]) {
		for (i = 0; hp->h_addr_list[i]; i++) {
			if ((cur = malloc_ai(port, ((struct in_addr *)hp->h_addr_list[i])->s_addr, socktype, proto)) == NULL) {
				if (*res)
					freeaddrinfo(*res);
				return EAI_MEMORY;
			}
			if (prev)
				prev->ai_next = cur;
			else
				*res = cur;
			prev = cur;
		}
		if (hints && hints->ai_flags & AI_CANONNAME && *res) {
			/* NOT sasl_strdup for compatibility */
			if (((*res)->ai_canonname = strdup(hp->h_name)) == NULL) {
				freeaddrinfo(*res);
				return EAI_MEMORY;
			}
		}
		return 0;
	}
	return EAI_NODATA;
}
