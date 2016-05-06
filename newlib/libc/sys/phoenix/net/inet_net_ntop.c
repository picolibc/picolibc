/*
 * Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* Author: Paul Vixie (ISC), July 1996 */
/* Copied from Linux, modified for Phoenix-RTOS. */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef SPRINTF_CHAR
# define SPRINTF(x) strlen(sprintf/**/x)
#else
# define SPRINTF(x) ((size_t)sprintf x)
#endif

static char *inet_net_ntop_ipv4(const u_char *src, int bits, char *dst, size_t size)
{
	char *odst = dst;
	char *t;
	u_int m;
	int b;

	if (bits < 0 || bits > 32) {
		errno = EINVAL;
		return (NULL);
	}

	if (bits == 0) {
		if (size < sizeof "0")
			goto emsgsize;

		*dst++ = '0';
		*dst = '\0';
	}

	/* Format whole octets. */
	for (b = bits / 8; b > 0; b--) {
		if (size < sizeof "255.")
			goto emsgsize;

		t = dst;
		dst += SPRINTF((dst, "%u", *src++));

		if (b > 1) {
			*dst++ = '.';
			*dst = '\0';
		}

		size -= (size_t)(dst - t);
	}

	/* Format partial octet. */
	b = bits % 8;

	if (b > 0) {
		if (size < sizeof ".255")
			goto emsgsize;

		t = dst;

		if (dst != odst)
			*dst++ = '.';

		m = ((1 << b) - 1) << (8 - b);
		dst += SPRINTF((dst, "%u", *src & m));
		size -= (size_t)(dst - t);
	}

	/* Format CIDR /width. */
	if (size < sizeof "/32")
		goto emsgsize;

	dst += SPRINTF((dst, "/%u", bits));
	return (odst);
emsgsize:
	errno = EMSGSIZE;
	return (NULL);
}

char *inet_net_ntop(int af, const void *netp, int bits, char *pres, size_t psize)
{
	switch (af) {
	case AF_INET:
		return inet_net_ntop_ipv4(netp, bits, pres, psize);

	default:
		errno = EAFNOSUPPORT;
		return NULL;
	}
}
