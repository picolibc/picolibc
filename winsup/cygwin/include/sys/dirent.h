/* Posix dirent.h for WIN32.

   Copyright 2001, 2002, 2003 Red Hat, Inc.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* Including this file should not require any Windows headers.  */
   
#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include <sys/types.h>

#define __DIRENT_VERSION	2

#pragma pack(push,4)
#ifdef __INSIDE_CYGWIN__
struct dirent
{
  long d_version;	/* Used since Cygwin 1.3.3. */
  __ino64_t d_ino;	/* still junk but with more bits */
  long d_fd;		/* File descriptor of open directory.
			   Used since Cygwin 1.3.3. */
  unsigned __ino32;
  char d_name[256];	/* FIXME: use NAME_MAX? */
};
#else
#ifdef __CYGWIN_USE_BIG_TYPES__
struct dirent
{
  long d_version;
  ino_t d_ino;
  long d_fd;
  unsigned long __ino32;
  char d_name[256];
};
#else
struct dirent
{
  long d_version;
  long d_reserved[2];
  long d_fd;
  ino_t d_ino;
  char d_name[256];
};
#endif
#endif
#pragma pack(pop)

#define __DIRENT_COOKIE 0xdede4242

#pragma pack(push,4)
typedef struct __DIR
{
  /* This is first to set alignment in non _COMPILING_NEWLIB case.  */
  unsigned long __d_cookie;
  struct dirent *__d_dirent;
  char *__d_dirname;		/* directory name with trailing '*' */
  _off_t __d_position;		/* used by telldir/seekdir */
  __ino64_t __d_dirhash;	/* hash of directory name for use by readdir */
  void *__handle;
  void *__fh;
  unsigned __flags;
} DIR;
#pragma pack(pop)

DIR *opendir (const char *);
struct dirent *readdir (DIR *);
void rewinddir (DIR *);
int closedir (DIR *);

int dirfd (DIR *);

#ifndef _POSIX_SOURCE
#ifndef __INSIDE_CYGWIN__
off_t telldir (DIR *);
void seekdir (DIR *, off_t loc);
#endif

int scandir (const char *__dir,
	     struct dirent ***__namelist,
	     int (*select) (const struct dirent *),
	     int (*compar) (const struct dirent **, const struct dirent **));

int alphasort (const struct dirent **__a, const struct dirent **__b);
#endif /* _POSIX_SOURCE */

#endif
