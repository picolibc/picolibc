/* netinet/in.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _NETINET_IN_H
#define _NETINET_IN_H

#include <cygwin/in.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if __MISC_VISIBLE
extern int bindresvport (int, struct sockaddr_in *);
extern int bindresvport_sa (int, struct sockaddr *);
#endif

#ifdef __cplusplus
};
#endif

#endif /* _NETINET_IN_H */
