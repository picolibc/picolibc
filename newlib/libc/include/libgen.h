/* Copyright (c) 2005 Jeff Johnston  <jjohnstn@redhat.com> */
/*
 * libgen.h - defined by XPG4
 */

#ifndef _LIBGEN_H_
#define _LIBGEN_H_

#include <sys/cdefs.h>

_BEGIN_STD_C

/* There are two common basename variants.  If you do NOT #include <libgen.h>
   and you do

     #define _GNU_SOURCE
     #include <string.h>

   you get the GNU version.  Otherwise you get the POSIX versionfor which you
   should #include <libgen.h>i for the function prototype.  POSIX requires that
   #undef basename will still let you invoke the underlying function.  However,
   this also implies that the POSIX version is used in this case.  That's made
   sure here. */
#undef basename
#define basename __xpg_basename
char      *basename (char *) __asm__(__ASMNAME("basename"));
char      *dirname (char *);

_END_STD_C

#endif /* _LIBGEN_H_ */

