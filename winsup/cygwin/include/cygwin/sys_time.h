/* sys_time.h

   Copyright 2005 Red Hat Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_SYS_TIME_H
#define _CYGWIN_SYS_TIME_H
#include <sys/select.h>

#ifdef __cplusplus
extern "C"
{
#endif

int futimes (int, const struct timeval *);
int lutimes (const char *, const struct timeval *);


#ifdef __cplusplus
}
#endif
#endif /*_CYGWIN_SYS_TIME_H*/
