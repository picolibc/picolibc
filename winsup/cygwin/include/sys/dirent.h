/* Posix dirent.h

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* Including this file should not require any Windows headers.  */

#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include <sys/cdefs.h>
#include <sys/types.h>
#include <limits.h>

#define __DIRENT_VERSION	3

/* Testing macros as per GLibC:
    _DIRENT_HAVE_D_NAMLEN == dirent has a d_namlen member
    _DIRENT_HAVE_D_OFF    == dirent has a d_off member
    _DIRENT_HAVE_D_RECLEN == dirent has a d_reclen member
    _DIRENT_HAVE_D_TYPE   == dirent has a d_type member
*/
#undef  _DIRENT_HAVE_D_NAMLEN
#undef  _DIRENT_HAVE_D_OFF
#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_TYPE

struct dirent
{
  __uint32_t	__d_version;			/* Used internally */
  ino_t		d_ino;
  unsigned char	d_type;
  unsigned char	__d_unused1[1];
  __uint16_t	d_reclen;
  __uint32_t	__d_internal1;
  char		d_name[NAME_MAX + 1];
};

#if __POSIX_VISIBLE >= 202405
#define DT_FORCE_TYPE	0x01	/* Suggested by SUS Base Specs Issue 8 */

typedef __uint16_t reclen_t;

/* This is a drop-in replacement for DIR, but used from posix_getdent()
   per SUS Base Specs Issue 8 */
struct posix_dent
{
  __uint32_t	__d_version;
  ino_t		d_ino;
  unsigned char	d_type;
  unsigned char	__d_unused1[1];
  reclen_t	d_reclen;
  __uint32_t	__d_internal1;
  char		d_name[NAME_MAX + 1];
};
#endif /* __POSIX_VISIBLE >= 202405 */

#define d_fileno d_ino			/* BSD compatible definition */

typedef struct __DIR DIR;

#if __BSD_VISIBLE || __POSIX_VISIBLE >= 202405
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
#endif /* __BSD_VISIBLE || __POSIX_VISIBLE >= 202405 */

#if __BSD_VISIBLE
/* Convert between stat structure types and directory types.  */
# define IFTODT(mode)		(((mode) & 0170000) >> 12)
# define DTTOIF(dirtype)        ((dirtype) << 12)
#endif /* __BSD_VISIBLE */
#endif /*_SYS_DIRENT_H*/
