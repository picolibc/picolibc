/* cygwin/_ucred.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN__UCRED_H
#define _CYGWIN__UCRED_H

#include <sys/types.h>

struct ucred {
  pid_t                 pid;
  uid_t                 uid;
  gid_t                 gid;
};

#endif /* _CYGWIN__UCRED_H */
