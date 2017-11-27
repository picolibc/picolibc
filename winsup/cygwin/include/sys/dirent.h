/* Posix dirent.h for WIN32.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* Including this file should not require any Windows headers.  */

#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include <sys/cdefs.h>
#include <sys/types.h>
#include <limits.h>

#define __DIRENT_VERSION	2

#ifdef __i386__
#pragma pack(push,4)
#endif
#define _DIRENT_HAVE_D_TYPE
struct dirent
{
  uint32_t __d_version;			/* Used internally */
  ino_t d_ino;
  unsigned char d_type;
  unsigned char __d_unused1[3];
  __uint32_t __d_internal1;
  char d_name[NAME_MAX + 1];
};
#ifdef __i386__
#pragma pack(pop)
#endif

#define d_fileno d_ino			/* BSD compatible definition */

#ifdef __x86_64__
#define __DIRENT_COOKIE 0xcdcd8484
#else
#define __DIRENT_COOKIE 0xdede4242
#endif

#ifdef __i386__
#pragma pack(push,4)
#endif
typedef struct __DIR
{
  /* This is first to set alignment in non _COMPILING_NEWLIB case.  */
  unsigned long __d_cookie;
  struct dirent *__d_dirent;
  char *__d_dirname;			/* directory name with trailing '*' */
  __int32_t __d_position;			/* used by telldir/seekdir */
  int __d_fd;
  uintptr_t __d_internal;
  void *__handle;
  void *__fh;
  unsigned __flags;
} DIR;
#ifdef __i386__
#pragma pack(pop)
#endif

DIR *opendir (const char *);
DIR *fdopendir (int);
struct dirent *readdir (DIR *);
int readdir_r (DIR * __restrict, struct dirent * __restrict,
	       struct dirent ** __restrict);
void rewinddir (DIR *);
int closedir (DIR *);

int dirfd (DIR *);

#if __MISC_VISIBLE || __XSI_VISIBLE
#ifndef __INSIDE_CYGWIN__
long telldir (DIR *);
void seekdir (DIR *, long loc);
#endif
#endif

#if __MISC_VISIBLE || __POSIX_VISIBLE >= 200809
int scandir (const char *__dir,
	     struct dirent ***__namelist,
	     int (*select) (const struct dirent *),
	     int (*compar) (const struct dirent **, const struct dirent **));
int alphasort (const struct dirent **__a, const struct dirent **__b);
#endif

#if __GNU_VISIBLE
int scandirat (int __dirfd, const char *__dir, struct dirent ***__namelist,
	       int (*select) (const struct dirent *),
	       int (*compar) (const struct dirent **, const struct dirent **));
int versionsort (const struct dirent **__a, const struct dirent **__b);
#endif

#if __BSD_VISIBLE
#ifdef _DIRENT_HAVE_D_TYPE
/* File types for `d_type'.  */
enum
{
  DT_UNKNOWN = 0,
# define DT_UNKNOWN     DT_UNKNOWN
  DT_FIFO = 1,
# define DT_FIFO        DT_FIFO
  DT_CHR = 2,
# define DT_CHR         DT_CHR
  DT_DIR = 4,
# define DT_DIR         DT_DIR
  DT_BLK = 6,
# define DT_BLK         DT_BLK
  DT_REG = 8,
# define DT_REG         DT_REG
  DT_LNK = 10,
# define DT_LNK         DT_LNK
  DT_SOCK = 12,
# define DT_SOCK        DT_SOCK
  DT_WHT = 14
# define DT_WHT         DT_WHT
};

/* Convert between stat structure types and directory types.  */
# define IFTODT(mode)		(((mode) & 0170000) >> 12)
# define DTTOIF(dirtype)        ((dirtype) << 12)
#endif /* _DIRENT_HAVE_D_TYPE */
#endif /* __BSD_VISIBLE */
#endif /*_SYS_DIRENT_H*/
