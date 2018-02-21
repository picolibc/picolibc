/* cygwin/_socketflags.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN__SOCKETFLAGS_H
#define _CYGWIN__SOCKETFLAGS_H

/* GNU extension flags.  Or them to the type parameter in calls to
   socket(2) to mark socket as nonblocking and/or close-on-exec. */
#define SOCK_NONBLOCK	0x01000000
#define SOCK_CLOEXEC	0x02000000
#ifdef __INSIDE_CYGWIN__
#define _SOCK_FLAG_MASK	0xff000000	/* Bits left for more extensions */
#endif

#endif /* _CYGWIN__SOCKETFLAGS_H */
