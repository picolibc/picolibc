/* minires.h.  Stub synchronous resolver for Cygwin.

   Copyright 2006 Red Hat, Inc.

   Written by Pierre A. Humblet <Pierre.Humblet@ieee.org>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/unistd.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>

extern in_addr_t cygwin_inet_addr (const char *);
extern int cygwin_socket (int, int, int);
extern int cygwin_bind (int, const struct sockaddr *, socklen_t);
extern int cygwin_connect (int, const struct sockaddr *, socklen_t);
extern int cygwin_select (int, fd_set *, fd_set *, fd_set *, struct timeval *);
extern int cygwin_sendto (int, const void *, size_t, int,
			  const struct sockaddr *, socklen_t);
extern int cygwin_recvfrom (int, void *, size_t, int, struct sockaddr *,
			    socklen_t *);

/* Number of elements is an array */
#define DIM(x) (sizeof(x) / sizeof(*(x)))

/* Definitions to parse the messages */
#define RD (1<<8) /* Offset in a short */
#define RA (1<<7)
#define QR (1<<7) /* Offsets in a char */
#define TC (1<<1)
#define ERR_MASK 0xF

/* Type for os specific res_lookup */
typedef int (os_query_t) (res_state, const char *, int, int, u_char *, int);

/* Special use of state elements */
#define sockfd _vcsock
#define mypid _flags
#define os_query qhook
#define use_os pfcode

#define DPRINTF(cond, format...)  if (cond) minires_dprintf(format)

/* Utility functions */
void minires_dprintf(const char * format, ...);
void minires_get_search(char * string, res_state statp);
void get_dns_info(res_state statp);
