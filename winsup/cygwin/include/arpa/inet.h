/* arpa/inet.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _ARPA_INET_H
#define _ARPA_INET_H

#include <netinet/in.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __INSIDE_CYGWIN_NET__
in_addr_t inet_addr (const char *);
int inet_aton (const char *, struct in_addr *);
in_addr_t inet_lnaof (struct in_addr);
struct in_addr inet_makeaddr (unsigned long , unsigned long);
in_addr_t inet_netof (struct in_addr);
in_addr_t inet_network (const char *);
char *inet_ntoa (struct in_addr);
int inet_pton (int, const char *, void *);
const char *inet_ntop (int, const void *, char *, socklen_t);
#endif

#ifdef __cplusplus
};
#endif

#endif /* _ARPA_INET_H */
