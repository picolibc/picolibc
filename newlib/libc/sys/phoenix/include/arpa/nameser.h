/* Copyright (c) 2016 Phoenix Systems
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.*/

#ifndef _ARPA_NAMESER_H
#define _ARPA_NAMESER_H

/* Define constants based on RFC 883, RFC 1034, RFC 1035 */
#define NS_PACKETSZ		512		/* Maximum packet size */
#define NS_MAXDNAME		1025	/* Maximum domain name */
#define NS_MAXCDNAME	255		/* Maximum compressed domain name */
#define NS_MAXLABEL		63		/* Maximum length of domain label */
#define NS_HFIXEDSZ		12		/* Bytes of fixed data in header */
#define NS_QFIXEDSZ		4		/* Bytes of fixed data in query */
#define NS_RRFIXEDSZ	10		/* Bytes of fixed data in r record */
#define NS_INT32SZ		4		/* Bytes of data in a u_int32_t */
#define NS_INT16SZ		2		/* Bytes of data in a u_int16_t */
#define NS_INT8SZ		1		/* Bytes of data in a u_int8_t */
#define NS_INADDRSZ		4		/* IPv4 T_A */
#define NS_IN6ADDRSZ	16		/* IPv6 T_AAAA */
#define NS_CMPRSFLGS	0xc0	/* Flag bits indicating name compression. */
#define NS_DEFAULTPORT	53		/* For both TCP and UDP. */

#endif
