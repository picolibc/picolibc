/* cygwin/grp.h

   Copyright 2002 Red Hat Inc.
   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_GRP_H_
#define _CYGWIN_GRP_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __INSIDE_CYGWIN__
struct __group16
{
  char *gr_name;
  char *gr_passwd;
  __gid16_t gr_gid;
  char **gr_mem;
};

struct __group32
{
  char *gr_name;
  char *gr_passwd;
  __gid32_t gr_gid;
  char **gr_mem;
};

struct __group32 * getgrgid32 (__gid32_t gid);
struct __group32 * getgrnam32 (const char *name);
__gid32_t getgid32 ();
__gid32_t getegid32 ();
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_GRP_H_ */
