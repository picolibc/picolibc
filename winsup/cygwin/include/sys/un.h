/* sys/un.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_UN_H
#define _SYS_UN_H

#include <sys/types.h>
#include <string.h>		/* for strlen */


/* POSIX requires only at least 100 bytes */
#define UNIX_PATH_MAX   108

struct sockaddr_un {
  __sa_family_t	 sun_family;              /* address family AF_LOCAL/AF_UNIX */
  char	         sun_path[UNIX_PATH_MAX]; /* 108 bytes of socket address     */
};

/* Evaluates the actual length of `sockaddr_un' structure. */
#define SUN_LEN(p) ((size_t)(((struct sockaddr_un *) NULL)->sun_path) \
		   + strlen ((p)->sun_path))

#endif
