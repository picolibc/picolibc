/* sys/utsname.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#ifdef __cplusplus
extern "C" {
#endif

#define _UTSNAME_LENGTH 65

struct utsname
{
  char sysname[_UTSNAME_LENGTH];
  char nodename[_UTSNAME_LENGTH];
  char release[_UTSNAME_LENGTH];
  char version[_UTSNAME_LENGTH];
  char machine[_UTSNAME_LENGTH];
#if __GNU_VISIBLE
  char domainname[_UTSNAME_LENGTH];
#else
  char __domainname[_UTSNAME_LENGTH];
#endif
};

int uname (struct utsname *);

#ifdef __cplusplus
}
#endif

#endif
