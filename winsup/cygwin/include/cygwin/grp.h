/* cygwin/grp.h
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

extern int getgrouplist (const char *, gid_t, gid_t *, int *);

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_GRP_H_ */
