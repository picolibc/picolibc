/* mntent.h

   Copyright 1996, 1998, 1999, 2000, 2001, 2006, 2009, 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _MNTENT_H
#define _MNTENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <paths.h>

struct mntent
{
  char *mnt_fsname;
  char *mnt_dir;
  char *mnt_type;
  char *mnt_opts;
  int mnt_freq;
  int mnt_passno;
};

#ifndef _NOMNTENT_FUNCS
#include <stdio.h>
FILE *setmntent (const char *__filep, const char *__type);
struct mntent *getmntent (FILE *__filep);
int endmntent (FILE *__filep);
#endif

/* The following two defines are deprecated.  Use the equivalent
   names from paths.h instead. */
#ifndef MNTTAB
#define MNTTAB _PATH_MNTTAB
#endif
/* This next file does exist, but the implementation of these
   functions does not actually use it.
   However, applications need the define to pass to setmntent().
*/
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif

#ifdef __cplusplus
};
#endif

#endif /* _MNTENT_H */
